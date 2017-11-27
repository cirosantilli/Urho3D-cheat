#include <iostream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

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
            auto ballDensity = 1.0f;

            // TODO: not working. Is there any way to avoid creating a custom
            // Component as in the ragdoll example?
            SubscribeToEvent(E_NODECOLLISION, URHO3D_HANDLER(Main, HandleNodeCollision));
            SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));
            SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));

            // Scene
            this->scene_ = new Scene(this->context_);
            this->scene_->CreateComponent<Octree>();
            this->scene_->CreateComponent<DebugRenderer>();
            this->scene_->CreateComponent<PhysicsWorld>();
            auto physicsWorld = scene_->GetComponent<PhysicsWorld>();
            physicsWorld->SetGravity(Vector2(0.0f, -10.0f));
            auto cache = GetSubsystem<ResourceCache>();

            // Graphics
            auto cameraNode_ = this->scene_->CreateChild("camera");
            cameraNode_->SetPosition(Vector3(0.0f, windowHeight / 4.0, -15.0f));
            auto camera = cameraNode_->CreateComponent<Camera>();
            camera->SetFarClip(500.0f);
            auto renderer = GetSubsystem<Renderer>();
            SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
            renderer->SetViewport(0, viewport);

            // Ground
            {
                auto node = scene_->CreateChild("Floor");
                node->SetPosition(Vector3(0.0f, -0.5f, 0.0f));
                node->SetScale(Vector3(1000.0f, 1.0f, 1000.0f));
                auto staticModel = node->CreateComponent<StaticModel>();
                staticModel->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                auto rigidBody = node->CreateComponent<RigidBody>();
                rigidBody->SetRestitution(1.0);
                auto collisionShape = node->CreateComponent<CollisionShape>();
                collisionShape->SetBox(Vector3::ONE);
            }

            // Falling balls
            {
                auto nodeLeft = this->scene_->CreateChild("Ball");
                {
                    auto& node = nodeLeft;
                    node->SetPosition(Vector3(-windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
                    auto staticModel = node->CreateComponent<StaticModel>();
                    staticModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
                    auto rigidBody = node->CreateComponent<RigidBody>();
                    rigidBody->SetMass(1.0f);
                    rigidBody->SetFriction(0.75f);
                    rigidBody->SetRestitution(ballRestitution);
                    auto collisionShape = node->CreateComponent<CollisionShape>();
                    collisionShape->SetSphere(2.0f * ballRadius);
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
            GetSubsystem<Renderer>()->DrawDebugGeometry(false);
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
