/*
TODO: why does the ball not bounce if the velocity <= 1.0?
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
        auto ballRestitution = 0.8f;
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
            this->groundNode = this->scene->CreateChild("Ground");
            this->groundNode->SetPosition(Vector3(0.0f, groundHeight / 2.0f, 0.0f));
            this->groundNode->CreateComponent<RigidBody2D>();
            auto collisionBox2d = this->groundNode->CreateComponent<CollisionBox2D>();
            collisionBox2d->SetSize(Vector2(groundWidth, groundHeight));
        }

        // Ball
        {
            this->ballNode = this->scene->CreateChild("LeftBall");
            this->ballNode->SetPosition(Vector3(0.0f, windowHeight / 2.0f, 0.0f));
            auto body = this->ballNode->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            body->SetLinearVelocity(Vector2(0.0f, -velocity));
            auto collisionCircle2d = this->ballNode->CreateComponent<CollisionCircle2D>();
            collisionCircle2d->SetRadius(ballRadius);
            collisionCircle2d->SetRestitution(ballRestitution);
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene;
    Node *ballNode, *groundNode;
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
