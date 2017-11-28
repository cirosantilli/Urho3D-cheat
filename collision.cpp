/*
Expected outcome: two falling balls. When either hits the ground, print a message to stdout.

https://stackoverflow.com/questions/47505166/how-to-detect-collisions-in-urho3d-in-a-2d-world
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
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include <Urho3D/Urho2D/CollisionCircle2D.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/PhysicsEvents2D.h>
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
        auto windowWidth = 10.0f;
        auto windowHeight = windowWidth;
        auto groundWidth = windowWidth;
        auto groundHeight = 1.0f;
        auto ballRadius = 0.5f;
        auto ballRestitution = 0.8f;

        // Events
        SubscribeToEvent(E_NODEUPDATECONTACT2D, URHO3D_HANDLER(Main, HandleNodeUpdateContact));
        SubscribeToEvent(E_PHYSICSBEGINCONTACT2D, URHO3D_HANDLER(Main, HandleNodeCollision));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));

        // Scene
        this->scene = new Scene(this->context_);
        this->scene->CreateComponent<Octree>();
        this->scene->CreateComponent<DebugRenderer>();
        this->scene->CreateComponent<PhysicsWorld2D>();
        auto physicsWorld = scene->GetComponent<PhysicsWorld2D>();
        physicsWorld->SetGravity(Vector2(0.0f, -10.0f));

        // Graphics
        auto cameraNode = this->scene->CreateChild("Camera");
        cameraNode->SetPosition(Vector3(0.0f, windowHeight / 2.0, -1.0f));
        auto camera = cameraNode->CreateComponent<Camera>();
        camera->SetOrthographic(true);
        camera->SetOrthoSize(windowWidth);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, this->scene, cameraNode->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Ground
        {
            auto node = this->scene->CreateChild("Ground");
            node->SetPosition(Vector3(0.0f, groundHeight / 2.0f, 0.0f));
            node->CreateComponent<RigidBody2D>();
            auto collisionBox2d = node->CreateComponent<CollisionBox2D>();
            collisionBox2d->SetSize(Vector2(groundWidth, groundHeight));
        }

        // Falling balls
        {
            this->leftBallNode = this->scene->CreateChild("LeftBall");
            {
                this->leftBallNode->SetPosition(Vector3(-windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
                auto body = this->leftBallNode->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                auto collisionCircle2d = this->leftBallNode->CreateComponent<CollisionCircle2D>();
                collisionCircle2d->SetRadius(ballRadius);
                collisionCircle2d->SetRestitution(ballRestitution);
            }
            this->rightBallNode = this->leftBallNode->Clone();
            this->rightBallNode->SetName("RightBall");
            this->rightBallNode->SetPosition(Vector3(windowWidth / 4.0f, windowHeight * (3.0f / 4.0f), 0.0f));
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene;
    Node *leftBallNode, *rightBallNode;
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData) {
        //auto node = static_cast<Node*>(eventData[NodeBeginContact2D::P_OTHERNODE].GetPtr());
        //auto s = node->GetName();
        std::cout << "asdf" << std::endl;
    }
    void HandleNodeUpdateContact(StringHash eventType, VariantMap& eventData) {
        //auto node = static_cast<Node*>(eventData[NodeBeginContact2D::P_OTHERNODE].GetPtr());
        //auto s = node->GetName();
        std::cout << "qwer" << std::endl;
    }
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        auto physicsWorld = this->scene->GetComponent<PhysicsWorld2D>();
        physicsWorld->DrawDebugGeometry();
    }
    void HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            engine_->Exit();
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
