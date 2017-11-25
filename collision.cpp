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
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include <Urho3D/Urho2D/CollisionCircle2D.h>
#include <Urho3D/Urho2D/Drawable2D.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>

using namespace Urho3D;

class Main : public Application {
    URHO3D_OBJECT(Main, Application);
public:
    Main(Context* context) : Application(context) {
    }
    virtual void Setup() override {
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_TITLE] = __FILE__;
        engineParameters_[EP_WINDOW_HEIGHT] = 512;
        engineParameters_[EP_WINDOW_WIDTH] = 512;
    }
    void Start() {
        // TODO: not working. Is there any way to avoid creating a custom
        // Component as in the ragdoll example?
        SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(Main, HandleNodeCollision));
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
        auto cameraNode_ = scene_->CreateChild("camera");
        cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -1.0f));
        auto camera = cameraNode_->CreateComponent<Camera>();
        camera->SetOrthographic(true);
        auto graphics = GetSubsystem<Graphics>();
        camera->SetOrthoSize(10.0);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Sprite cache
        auto cache = GetSubsystem<ResourceCache>();
        auto boxSprite = cache->GetResource<Sprite2D>("Urho2D/Box.png");
        auto ballSprite = cache->GetResource<Sprite2D>("Urho2D/Ball.png");

        // Ground
        {
            auto h = 2.0;
            auto w = 2.0;
            auto node = scene_->CreateChild("ground");
            node->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            auto staticSprite = node->CreateComponent<StaticSprite2D>();
            auto rect = boxSprite->GetRectangle();
            auto scaleX = PIXEL_SIZE * rect.Width();
            auto scaleY = PIXEL_SIZE * rect.Height();
            node->SetScale(Vector3(w / scaleX, h / scaleY, 0.0f));
            node->CreateComponent<RigidBody2D>();
            //staticSprite->SetSprite(boxSprite);
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(scaleX, scaleY));
        }

        // Falling circle
        {
            auto r = 2.0;
            auto node  = scene_->CreateChild("circle");
            node->SetPosition(Vector3(0.0f, 4.0f, 0.0f));
            auto staticSprite = node->CreateComponent<StaticSprite2D>();
            auto rect = boxSprite->GetRectangle();
            auto scaleX = PIXEL_SIZE * rect.Width();
            auto scaleY = PIXEL_SIZE * rect.Height();
            node->SetScale(Vector3(r / scaleX, r / scaleY, 0.0f));
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            //staticSprite->SetSprite(ballSprite);
            auto circle = node->CreateComponent<CollisionCircle2D>();
            circle->SetRadius(scaleX / 2.0);
            circle->SetDensity(1.0f);
            circle->SetRestitution(0.8f);
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene_;
    void HandleNodeCollision(StringHash eventType, VariantMap& eventData) {
        std::cout << "asdf" << std::endl;
    }
	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
		auto* physicsWorld = this->scene_->GetComponent<PhysicsWorld2D>();
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
