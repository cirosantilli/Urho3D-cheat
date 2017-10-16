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
    }
	void Start() {
		// Scene
		scene_ = new Scene(this->context_);
		scene_->CreateComponent<Octree>();
		scene_->CreateComponent<DebugRenderer>();
		scene_->CreateComponent<PhysicsWorld2D>();

		// Graphics
		auto cameraNode_ = scene_->CreateChild("camera");
		cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -1.0f));
		auto camera = cameraNode_->CreateComponent<Camera>();
		camera->SetOrthographic(true);
		auto graphics = GetSubsystem<Graphics>();
		camera->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);
		auto renderer = GetSubsystem<Renderer>();
		SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
		renderer->SetViewport(0, viewport);

		// Sprite cache
		auto cache = GetSubsystem<ResourceCache>();
		auto boxSprite = cache->GetResource<Sprite2D>("Urho2D/Box.png");
		auto ballSprite = cache->GetResource<Sprite2D>("Urho2D/Ball.png");
		std::cout << boxSprite->GetRectangle().left_ << std::endl;
		std::cout << boxSprite->GetRectangle().right_ << std::endl;

		// Ground
		{
			auto node = scene_->CreateChild("ground");
			node->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
			node->SetScale(Vector3(1/0.32f, 1.0f, 0.0f));
			node->CreateComponent<RigidBody2D>();
			auto staticSprite = node->CreateComponent<StaticSprite2D>();
			staticSprite->SetSprite(boxSprite);
			auto shape = node->CreateComponent<CollisionBox2D>();
			shape->SetSize(Vector2(0.32f, 0.32f));
		}

		// Falling circle
		{
			auto node  = scene_->CreateChild("circle");
			node->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
			node->SetScale(1.0f/0.16f);
			auto body = node->CreateComponent<RigidBody2D>();
			body->SetBodyType(BT_DYNAMIC);
			auto staticSprite = node->CreateComponent<StaticSprite2D>();
			staticSprite->SetSprite(ballSprite);
			auto circle = node->CreateComponent<CollisionCircle2D>();
			circle->SetRadius(0.16f);
			circle->SetDensity(1.0f);
			circle->SetRestitution(0.8f);
		}
	}
	void Stop() {}
private:
	SharedPtr<Scene> scene_;
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
