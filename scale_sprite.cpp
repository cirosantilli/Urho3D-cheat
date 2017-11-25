/*
https://stackoverflow.com/questions/47488411/how-to-scale-a-sprite2d-in-urho3d-without-rescaling-the-entire-node
*/

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
    }
    void Start() {
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));
        this->scene_ = new Scene(this->context_);
        scene_->CreateComponent<Octree>();
        scene_->CreateComponent<DebugRenderer>();
        scene_->CreateComponent<PhysicsWorld2D>();
        auto physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
        auto cameraNode_ = scene_->CreateChild("camera");
        cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -1.0f));
        auto camera = cameraNode_->CreateComponent<Camera>();
        camera->SetOrthographic(true);
        camera->SetOrthoSize(4.0);
        auto graphics = GetSubsystem<Graphics>();
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);
        auto cache = GetSubsystem<ResourceCache>();
        auto boxSprite = cache->GetResource<Sprite2D>("Urho2D/Box.png");

		auto groundWidth = 2.0;
		auto groundHeight = 2.0;
		auto node = this->scene_->CreateChild("ground");
		node->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
		node->CreateComponent<RigidBody2D>();
		auto shape = node->CreateComponent<CollisionBox2D>();
#if 0
		// Sprite and collision have the same size,
		// but I feel this is very convoluted.
		auto rect = boxSprite->GetRectangle();
		auto scaleX = PIXEL_SIZE * rect.Width();
		auto scaleY = PIXEL_SIZE * rect.Height();
		node->SetScale(Vector3(groundWidth / scaleX, groundHeight / scaleY, 0.0f));
		shape->SetSize(Vector2(scaleX, scaleY));
#else
		// Collision shape is correct, but the sprite is tiny,
		// so not the behaviour that I want.
		// But closer to the code that I would want to write.
		shape->SetSize(Vector2(groundWidth, groundHeight));
#endif
		auto staticSprite = node->CreateComponent<StaticSprite2D>();
		staticSprite->SetSprite(boxSprite);
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene_;
	void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
		auto physicsWorld = this->scene_->GetComponent<PhysicsWorld2D>();
		physicsWorld->DrawDebugGeometry();
	}
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
