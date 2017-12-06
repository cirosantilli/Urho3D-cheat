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
    float windowWidth = 1.0f;
    void Start() {
        auto windowHeight =this->windowWidth;
        auto wallLength =this->windowWidth;
        auto wallWidth =this->windowWidth / 20.0f;
        auto ballRadius =this->windowWidth / 20.0f;
        auto ballRestitution = 1.0f;
        auto playerLength = windowHeight / 4.0f;
        this->score = 0;
        this->input = this->GetSubsystem<Input>();

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
        this->physicsWorld = this->scene->GetComponent<PhysicsWorld2D>();

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

        Node *wallNode;
        {
            wallNode = this->scene->CreateChild("BottomWall");
            wallNode->SetPosition(Vector3(this->windowWidth / 2.0, wallWidth / 2.0f, 0.0f));
            wallNode->CreateComponent<RigidBody2D>();
            auto collisionBox2d = wallNode->CreateComponent<CollisionBox2D>();
            collisionBox2d->SetSize(Vector2(wallLength, wallWidth));
            collisionBox2d->SetRestitution(0.0);
        }

        {
            auto node = wallNode->Clone();
            node->SetName("TopWall");
            node->SetPosition(Vector3(this->windowWidth / 2.0f, windowHeight - (wallWidth / 2.0f), 0.0f));
        }

        {
            auto& node = this->rightWallNode;
            node = wallNode->Clone();
            node->SetName("RightWall");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector3(this->windowWidth - (wallWidth / 2.0f), windowHeight / 2.0f, 0.0f));
        }

        {
            auto& node = this->leftWallNode;
            node = wallNode->Clone();
            node->SetName("LeftWall");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector3(-wallWidth / 2.0f, windowHeight / 2.0f, 0.0f));
        }

        {
            auto& node = this->playerNode;
            node = wallNode->Clone();
            node->SetName("Player");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector3(wallWidth / 2.0f, windowHeight / 2.0f, 0.0f));
            auto body = node->GetComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            body->SetFixedRotation(true);
            auto shape = node->GetComponent<CollisionBox2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(1.0);
            shape->SetSize(Vector2(playerLength, wallWidth));
        }


        // Ball
        {
            this->ballNode = this->scene->CreateChild("Ball");
            this->ballNode->SetPosition(Vector3(this->windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
            auto body = this->ballNode->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            body->SetLinearVelocity(Vector2(2 * this->windowWidth, -windowHeight / 2.0f));
            auto shape = this->ballNode->CreateComponent<CollisionCircle2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRadius(ballRadius);
            shape->SetRestitution(ballRestitution);
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene;
    Node *ballNode, *playerNode, *leftWallNode, *rightWallNode;
    Text *text;
    Input *input;
    PhysicsWorld2D *physicsWorld;
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
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        Node *otherNode;
        bool isBall = false;
        if (nodea == this->ballNode) {
            otherNode = nodeb;
            isBall = true;
        }
        if (nodeb == this->ballNode) {
            otherNode = nodea;
            isBall = true;
        }
        if (otherNode == this->rightWallNode) {
            this->score++;
        } else if (otherNode == this->leftWallNode) {
            this->score = 0;
        }
        this->text->SetText(std::to_string(this->score).c_str());
    }
    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        using namespace Update;
        auto timeStep = eventData[P_TIMESTEP].GetFloat();
        auto impuseMagnitude = (this->windowWidth * 2.0f) * timeStep;
        auto body = this->playerNode->GetComponent<RigidBody2D>();
        if (this->input->GetKeyDown(KEY_DOWN)) {
            body->ApplyForceToCenter(Vector2(0.0f, -impuseMagnitude), true);
        } else if (this->input->GetKeyDown(KEY_UP)) {
            body->ApplyForceToCenter(Vector2(0.0f, impuseMagnitude), true);
        } else if (this->input->GetKeyDown(KEY_RIGHT)) {
            body->ApplyForceToCenter(Vector2(impuseMagnitude, 0.0f), true);
        } else if (input->GetKeyDown(KEY_LEFT)) {
            body->ApplyForceToCenter(Vector2(-impuseMagnitude, 0.0f), true);
        }
    }
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        this->physicsWorld->DrawDebugGeometry();
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
