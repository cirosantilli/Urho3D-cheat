/*
Single player pong.
*/

#include <iostream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include <Urho3D/Urho2D/CollisionCircle2D.h>
#include <Urho3D/Urho2D/PhysicsEvents2D.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>

using namespace Urho3D;

class Main : public Application {
    URHO3D_OBJECT(Main, Application);
public:
    Main(Context* context) : Application(context) {}
    virtual void Setup() override {
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_TITLE] = __FILE__;
        engineParameters_[EP_WINDOW_HEIGHT] = 512;
        engineParameters_[EP_WINDOW_WIDTH] = 512;
    }
    void Start() {
        auto windowWidth = 1.0f;
        auto windowHeight = windowWidth;
        auto wallLength = windowWidth;
        auto wallWidth = windowWidth / 20.0f;
        auto ballRadius = windowWidth / 20.0f;
        auto ballRestitution = 1.0f;
        auto playerLength = windowHeight / 4.0f;
        this->score = 0;

        // Events
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));
        SubscribeToEvent(E_PHYSICSBEGINCONTACT2D, URHO3D_HANDLER(Main, HandlePhysicsBeginContact2D));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));

        // Scene
        this->scene = new Scene(this->context_);
        this->scene->CreateComponent<Octree>();
        this->scene->CreateComponent<DebugRenderer>();
        this->scene->CreateComponent<PhysicsWorld2D>();
        auto physicsWorld = scene->GetComponent<PhysicsWorld2D>();
        physicsWorld->SetGravity(Vector2(0.0f, 0.0f));

        // Graphics
        auto cameraNode = this->scene->CreateChild("Camera");
        cameraNode->SetPosition(Vector3(windowWidth / 2.0f, windowHeight / 2.0f, -1.0f));
        auto camera = cameraNode->CreateComponent<Camera>();
        camera->SetOrthographic(true);
        camera->SetOrthoSize(windowWidth);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, this->scene, cameraNode->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Score
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        auto ui = GetSubsystem<UI>();
        this->text = ui->GetRoot()->CreateChild<Text>();
        this->text->SetText(std::to_string(this->score).c_str());
        this->text->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
        this->text->SetHorizontalAlignment(HA_CENTER);
        this->text->SetVerticalAlignment(VA_CENTER);

        {
            this->wallNode = this->scene->CreateChild("BottomWall");
            this->wallNode->SetPosition(Vector3(windowWidth / 2.0, wallWidth / 2.0f, 0.0f));
            this->wallNode->CreateComponent<RigidBody2D>();
            auto collisionBox2d = this->wallNode->CreateComponent<CollisionBox2D>();
            collisionBox2d->SetSize(Vector2(wallLength, wallWidth));
            collisionBox2d->SetRestitution(0.0);
        }

        {
            auto node = this->wallNode->Clone();
            node->SetName("TopWall");
            node->SetPosition(Vector3(windowWidth / 2.0f, windowHeight - (wallWidth / 2.0f), 0.0f));
        }

        {
            auto node = this->wallNode->Clone();
            node->SetName("RightWall");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector3(windowWidth - (wallWidth / 2.0f), windowHeight / 2.0f, 0.0f));
        }

        //{
            //auto node = this->wallNode->Clone();
            //node->SetName("LeftWall");
            //node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            //node->SetPosition(Vector3(-wallWidth / 2.0f, windowHeight / 2.0f, 0.0f));
        //}

        {
            this->playerNode = this->wallNode->Clone();
            this->playerNode->SetName("Player");
            this->playerNode->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            this->playerNode->SetPosition(Vector3(wallWidth / 2.0f, windowHeight / 2.0f, 0.0f));
            auto body = this->playerNode->GetComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            body->SetMass(1.0);
            auto box = this->playerNode->GetComponent<CollisionBox2D>();
            box->SetSize(Vector2(playerLength, wallWidth));
            box->SetRestitution(1.0);
        }


        // Ball
        {
            this->ballNode = this->scene->CreateChild("Ball");
            this->ballNode->SetPosition(Vector3(windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
            auto body = this->ballNode->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            body->SetLinearVelocity(Vector2(0.0f, -1.01f));
            auto collisionCircle2d = this->ballNode->CreateComponent<CollisionCircle2D>();
            collisionCircle2d->SetRadius(ballRadius);
            collisionCircle2d->SetRestitution(ballRestitution);
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene;
    Node *ballNode, *playerNode, *wallNode;
    Text *text;
    uint64_t score;
    void HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            engine_->Exit();
        }
    }
    void HandlePhysicsBeginContact2D(StringHash eventType, VariantMap& eventData) {
        using namespace PhysicsBeginContact2D;

        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        std::cout << "node a name " << nodea->GetName().CString() << std::endl;
        if (nodea == this->ballNode) {
            std::cout << "node a == ballNode" << std::endl;
        } else if (nodea == this->wallNode) {
            std::cout << "node a == wallNode" << std::endl;
        }

        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        std::cout << "node b name " << nodeb->GetName().CString() << std::endl;
        if (nodeb == this->ballNode) {
            std::cout << "node b == ballNode" << std::endl;
        } else if (nodeb == this->ballNode) {
            std::cout << "node b == wallNode" << std::endl;
        }

        //if (
            //nodea == this->ballNode && nodeb == this->rightWallNode ||
            //nodeb == this->ballNode and nodeb == this->rightWallNode
        //) {
            this->score++;
        //}
        this->text->SetText(std::to_string(this->score).c_str());

        std::cout << std::endl;
    }
    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        auto input = GetSubsystem<Input>();
        auto impuseMagnitude = 0.01f;
        auto body = this->playerNode->GetComponent<RigidBody2D>();
        if (input->GetKeyDown(KEY_DOWN)) {
            body->ApplyLinearImpulseToCenter(Vector2(0.0f, -impuseMagnitude), true);
            std::cout << "down" << std::endl;
        } else if (input->GetKeyDown(KEY_UP)) {
            body->ApplyLinearImpulseToCenter(Vector2(0.0f, impuseMagnitude), true);
            std::cout << "up" << std::endl;
        } else if (input->GetKeyDown(KEY_RIGHT)) {
            body->ApplyLinearImpulseToCenter(Vector2(impuseMagnitude, 0.0f), true);
            std::cout << "right" << std::endl;
        } else if (input->GetKeyDown(KEY_LEFT)) {
            body->ApplyLinearImpulseToCenter(Vector2(-impuseMagnitude, 0.0f), true);
            std::cout << "left" << std::endl;
        }
    }
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        auto physicsWorld = this->scene->GetComponent<PhysicsWorld2D>();
        physicsWorld->DrawDebugGeometry();
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
