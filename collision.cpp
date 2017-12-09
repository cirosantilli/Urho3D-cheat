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
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
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
        // 1.0f does not bounce back as explained in velocity_stop.cpp
        auto windowWidth = 10.0f;
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
        physicsWorld = this->scene->CreateComponent<PhysicsWorld2D>();
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

        // Ground
        {
            auto& node = this->groundNode;
            node = this->scene->CreateChild("Ground");
            node->SetPosition(Vector3(0.0f, groundHeight / 2.0f, 0.0f));
            node->CreateComponent<RigidBody2D>();
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(groundWidth, groundHeight));
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        }

        // Left ball
        {
            this->leftBallNode = this->scene->CreateChild("LeftBall");
            this->leftBallNode->SetPosition(Vector3(-windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
            auto body = this->leftBallNode->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            auto shape = this->leftBallNode->CreateComponent<CollisionCircle2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRadius(ballRadius);
            shape->SetRestitution(0.75f);
        }

        // Right ball
        {
            auto& node = this->rightBallNode;
            node = this->leftBallNode->Clone();
            node->SetName("RightBall");
            node->SetPosition(Vector3(windowWidth / 4.0f, windowHeight * (3.0f / 4.0f), 0.0f));
        }
    }
    void Stop() {}
private:
    Node *leftBallNode, *groundNode, *rightBallNode;
    PhysicsWorld2D *physicsWorld;
    SharedPtr<Scene> scene;

    /// Mostly to see if the simulation is deterministic.
    uint64_t steps;
    void HandleUpdate(StringHash /*eventType*/, VariantMap& eventData) {
        this->steps++;
    }
    void HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            engine_->Exit();
        } else if (key == KEY_R) {
            this->scene->Clear();
            this->Start();
        } else if (key == KEY_F5) {
            File saveFile(this->context_, GetSubsystem<FileSystem>()->GetProgramDir() + "save.xml", FILE_WRITE);
            this->scene->SaveXML(saveFile);
        } else if (key == KEY_F7) {
            // TODO does not work, things disappear from screen. Why?
            File saveFile(this->context_, GetSubsystem<FileSystem>()->GetProgramDir() + "save.xml", FILE_READ);
            this->scene->LoadXML(saveFile);
        }
    }
    void HandlePhysicsBeginContact2D(StringHash eventType, VariantMap& eventData) {
        using namespace PhysicsBeginContact2D;

        std::cout << "steps " << this->steps << std::endl;

        // nodea
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        std::cout << "node a name " << nodea->GetName().CString() << std::endl;
        if (nodea == this->leftBallNode) {
            std::cout << "node a == leftBallNode" << std::endl;
        } else if (nodea == this->rightBallNode) {
            std::cout << "node a == rightBallNode" << std::endl;
        } else if (nodea == this->groundNode) {
            std::cout << "node a == groundNode" << std::endl;
        }

        // nodeb
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        std::cout << "node b name " << nodeb->GetName().CString() << std::endl;
        if (nodeb == this->leftBallNode) {
            std::cout << "node b == leftBallNode" << std::endl;
        } else if (nodeb == this->rightBallNode) {
            std::cout << "node b == rightBallNode" << std::endl;
        } else if (nodeb == this->groundNode) {
            std::cout << "node b == groundNode" << std::endl;
        }

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
        this->physicsWorld->DrawDebugGeometry();
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
