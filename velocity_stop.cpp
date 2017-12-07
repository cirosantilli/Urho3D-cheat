/*
We have to hack b2Settings::b2_velocityThreshold in the Box2D source to something below 1.0
for the 1.0 velocity to bounce back. 1.01 bounces back fine.

- https://gamedev.stackexchange.com/questions/84737/why-do-my-box2d-bodies-occasionally-get-stuck-and-separate-forcibly/151878#151878
- https://stackoverflow.com/questions/5381399/how-can-i-prevent-a-ball-from-sticking-to-walls-in-box2d
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
#include <Urho3D/IO/MemoryBuffer.h>
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
        auto velocity = 1.00f;
        //auto velocity = 1.01f;
        auto windowWidth = 1.0f;
        auto windowHeight = windowWidth;
        auto groundWidth = windowWidth;
        auto groundHeight = windowWidth / 10.0f;
        auto ballRadius = windowWidth / 20.0f;
        auto ballRestitution = 1.0f;
        this->steps = 0;

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
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(groundWidth, groundHeight));
            shape->SetDensity(1.0);
            shape->SetFriction(1.0f);
            shape->SetRestitution(1.0);
        }

        // Ball
        {
            auto node = this->scene->CreateChild("Ball");
            node->SetPosition(Vector3(0.0f, windowHeight / 2.0f, 0.0f));
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            body->SetLinearVelocity(Vector2(0.0f, -velocity));
            auto shape = node->CreateComponent<CollisionCircle2D>();
            shape->SetDensity(1.0);
            shape->SetFriction(1.0f);
            shape->SetRadius(ballRadius);
            shape->SetRestitution(1.0);
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene;
    uint64_t steps;
    void HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            engine_->Exit();
        }
    }
    void HandleUpdate(StringHash /*eventType*/, VariantMap& eventData) {
        this->steps++;
    }
    void HandlePhysicsBeginContact2D(StringHash eventType, VariantMap& eventData) {
        using namespace PhysicsBeginContact2D;
        std::cout << "steps " << std::endl;

        // nodea
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        std::cout << "node a name " << nodea->GetName().CString() << std::endl;

        // nodeb
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        std::cout << "node b name " << nodeb->GetName().CString() << std::endl;

        // Contacts
        MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
        while (!contacts.IsEof()) {
            auto contactPosition = contacts.ReadVector2();
            auto contactNormal = contacts.ReadVector2();
            auto contactDistance = contacts.ReadFloat();
            auto contactImpulse = contacts.ReadFloat();
            std::cout << "contact position " << contactPosition.ToString().CString() << std::endl;
            std::cout << "contact normal " << contactNormal.ToString().CString() << std::endl;
            std::cout << "contact distance " << contactDistance << std::endl;
            std::cout << "contact impulse " << contactImpulse << std::endl;
        }

        std::cout << std::endl;
    }
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        auto physicsWorld = this->scene->GetComponent<PhysicsWorld2D>();
        physicsWorld->DrawDebugGeometry();
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
