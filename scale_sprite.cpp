/*
Expected outcome: two falling boxes on a ground. Box sprites are correctly scaled to match the physical bodies.

- https://stackoverflow.com/questions/47488411/how-to-scale-a-sprite2d-in-urho3d-without-rescaling-the-entire-node
- https://discourse.urho3d.io/t/how-to-scale-a-sprite2d-in-urho3d-without-rescaling-the-entire-node/3785
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
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include <Urho3D/Urho2D/CollisionCircle2D.h>
#include <Urho3D/Urho2D/Drawable2D.h>
#include <Urho3D/Urho2D/PhysicsEvents2D.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>

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
        auto groundHeight = windowWidth / 10.0f;

        // Events
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));

        // Scene
        this->scene = new Scene(this->context_);
        this->scene->CreateComponent<Octree>();
        this->scene->CreateComponent<DebugRenderer>();
        this->scene->CreateComponent<PhysicsWorld2D>();
        auto physicsWorld = scene->GetComponent<PhysicsWorld2D>();
        physicsWorld->SetGravity(Vector2(0.0f, -windowWidth));

        // Graphics
        auto cameraNode = this->scene->CreateChild("Camera");
        cameraNode->SetPosition(Vector3(0.0f, windowHeight / 2.0, -1.0f));
        auto camera = cameraNode->CreateComponent<Camera>();
        camera->SetOrthographic(true);
        camera->SetOrthoSize(windowWidth);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, this->scene, cameraNode->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);
        auto cache = GetSubsystem<ResourceCache>();
        this->boxSprite = cache->GetResource<Sprite2D>("Urho2D/Box.png");

        // Ground
        {
            auto& node = this->groundNode;
            node = this->scene->CreateChild("Ground");
            node->SetPosition(Vector3(0.0f, groundHeight / 2.0f, 0.0f));
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_STATIC);
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        }

        // Boxes
        this->CreateBox(0.0, windowWidth / 2.0f, windowWidth /  5.0, windowWidth / 10.0);
        this->CreateBox(0.0, windowWidth / 4.0f, windowWidth / 10.0, windowWidth / 10.0);
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene;
    Node *groundNode;
    Sprite2D *boxSprite;
    void HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            engine_->Exit();
        }
    }
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        auto physicsWorld = this->scene->GetComponent<PhysicsWorld2D>();
        physicsWorld->DrawDebugGeometry();
    }
    void CreateBox(float x, float y, float width, float height) {
        auto node = this->scene->CreateChild("Box");
        node->SetPosition(Vector3(x, y, 0.0f));
        node->SetRotation(Quaternion(0.0f, 0.0f, 30.0f));
        auto box = node->CreateComponent<CollisionBox2D>();
        box->SetDensity(1.0f);
        box->SetFriction(1.0f);
        box->SetRestitution(0.75f);
        box->SetSize(Vector2(width, height));
        auto body = node->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_DYNAMIC);
        auto staticSprite = node->CreateComponent<StaticSprite2D>();
        staticSprite->SetSprite(this->boxSprite);
        staticSprite->SetDrawRect(Rect(
            width / 2.0f,
            -height / 2.0f,
            -width / 2.0f,
            height / 2.0f
        ));
        staticSprite->SetUseDrawRect(true);
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
