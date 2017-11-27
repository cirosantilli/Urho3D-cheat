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
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include <Urho3D/Urho2D/CollisionCircle2D.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/PhysicsEvents2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>

#include <Urho3D/Scene/Component.h>

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
        SubscribeToEvent(E_PHYSICSBEGINCONTACT2D, URHO3D_HANDLER(Main, HandleNodeCollision));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));

        // Scene
        this->scene_ = new Scene(this->context_);
        this->scene_->CreateComponent<Octree>();
        this->scene_->CreateComponent<DebugRenderer>();
        this->scene_->CreateComponent<PhysicsWorld2D>();
        auto physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
        physicsWorld->SetGravity(Vector2(0.0f, -10.0f));

        // Graphics
        auto cameraNode_ = this->scene_->CreateChild("Camera");
        // Center of the camera.
        cameraNode_->SetPosition(Vector3(0.0f, windowHeight / 2.0, -1.0f));
        auto camera = cameraNode_->CreateComponent<Camera>();
        camera->SetOrthographic(true);
        camera->SetOrthoSize(windowWidth);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, this->scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Ground
        {
            auto node = this->scene_->CreateChild("Ground");
            node->SetPosition(Vector3(0.0f, groundHeight / 2.0f, 0.0f));
            node->CreateComponent<RigidBody2D>();
            auto collisionBox2d = node->CreateComponent<CollisionBox2D>();
            collisionBox2d->SetSize(Vector2(groundWidth, groundHeight));
        }

        // Falling balls
        {
            auto nodeLeft  = this->scene_->CreateChild("BallLeft");
            {
                auto& node = nodeLeft;
                node->SetPosition(Vector3(-windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
                auto body = node->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                auto collisionCircle2d = node->CreateComponent<CollisionCircle2D>();
                collisionCircle2d->SetRadius(ballRadius);
                collisionCircle2d->SetRestitution(ballRestitution);
            }
            auto nodeRight = nodeLeft->Clone();
            nodeRight->SetPosition(Vector3(windowWidth / 4.0f, windowHeight * (3.0f / 4.0f), 0.0f));
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene_;
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData) {
        std::cout << "asdf" << std::endl;
    }
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        auto physicsWorld = this->scene_->GetComponent<PhysicsWorld2D>();
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
