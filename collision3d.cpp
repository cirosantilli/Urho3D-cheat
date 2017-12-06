/*
Expected outcome: two falling balls. When either hits the ground, print a message to stdout.
*/

#include <cassert>
#include <iostream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>

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
        // TODO: balls float midair if 1.0f.
        // See: velocity_stop.cpp for a minimzed 2D version.
        auto windowWidth = 10.0f;
        auto windowHeight = windowWidth;
        auto groundWidth = windowWidth;
        auto groundHeight = windowWidth / 10.0f;
        auto ballRadius = windowWidth / 20.0f;
        auto ballRestitution = 0.75f;

        // Events
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));
        SubscribeToEvent(E_NODECOLLISIONSTART, URHO3D_HANDLER(Main, HandleNodeCollisionStart));
        SubscribeToEvent(E_PHYSICSCOLLISIONSTART, URHO3D_HANDLER(Main, HandlePhysicsCollisionStart));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));

        // Scene
        this->scene = new Scene(this->context_);
        this->scene->CreateComponent<Octree>();
        this->scene->CreateComponent<DebugRenderer>();
        this->scene->CreateComponent<PhysicsWorld>();
        auto physicsWorld = scene->GetComponent<PhysicsWorld>();
        physicsWorld->SetGravity(Vector2(0.0f, -windowWidth));
        auto cache = GetSubsystem<ResourceCache>();

        // Graphics
        auto cameraNode = this->scene->CreateChild("Camera");
        cameraNode->SetPosition(Vector3(0.0f, windowHeight / 4.0, -1.5 * windowWidth));
        auto camera = cameraNode->CreateComponent<Camera>();
        camera->SetFarClip(500.0f);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene, cameraNode->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Ground
        {
            auto& node = this->groundNode;
            node = scene->CreateChild("Ground");
            node->SetPosition(Vector3(0.0f, -groundHeight / 2.0, 0.0f));
            node->SetScale(Vector3(groundWidth, groundHeight, groundWidth));
            auto body = node->CreateComponent<RigidBody>();
            body->SetRestitution(1.0);
            auto shape = node->CreateComponent<CollisionShape>();
            shape->SetBox(Vector3::ONE);
        }

        // Left ball
        {
            auto& node = this->leftBallNode;
            node = this->scene->CreateChild("LeftBall");
            node->SetPosition(Vector3(-windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
            auto body = node->CreateComponent<RigidBody>();
            body->SetRestitution(ballRestitution);
            body->SetMass(1.0f);
            auto shape = node->CreateComponent<CollisionShape>();
            shape->SetSphere(2.0f * ballRadius);
        }

        // Right ball
        {
            auto& node = this->rightBallNode;
            node = leftBallNode->Clone();
            node->SetName("RightBall");
            node->SetPosition(Vector3(windowWidth / 4.0f, windowHeight * (3.0f / 4.0f), 0.0f));
        }
    }
    void Stop() {}
private:
    SharedPtr<Scene> scene;
    Node *leftBallNode, *rightBallNode, *groundNode;
    void HandleNodeCollisionStart(StringHash eventType, VariantMap& eventData) {
        using namespace NodeCollisionStart;
        std::cout << "HandleNodeCollisionStart" << std::endl;
        assert(eventType == E_NODECOLLISIONSTART);
        auto body = static_cast<RigidBody*>(eventData[P_BODY].GetPtr());
        std::cout << "this body mass " << body->GetMass() << std::endl;
        auto otherBody = static_cast<RigidBody*>(eventData[P_OTHERBODY].GetPtr());
        std::cout << "other body mass " << otherBody->GetMass() << std::endl;
        auto otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
        std::cout << "other node name " << otherNode->GetName().CString() << std::endl;
        auto trigger = static_cast<bool>(eventData[P_TRIGGER].GetPtr());
        std::cout << "trigger " << trigger << std::endl;
        MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
        while (!contacts.IsEof()) {
            Vector3 contactPosition = contacts.ReadVector3();
            Vector3 contactNormal = contacts.ReadVector3();
            float contactDistance = contacts.ReadFloat();
            float contactImpulse = contacts.ReadFloat();
            std::cout << "contact position " << contactPosition.ToString().CString() << std::endl;
            std::cout << "contact normal " << contactNormal.ToString().CString() << std::endl;
            std::cout << "contact distance " << contactDistance << std::endl;
            std::cout << "contact impulse " << contactImpulse << std::endl;
        }
        std::cout << std::endl;

    }
    void HandlePhysicsCollisionStart(StringHash eventType, VariantMap& eventData) {
        using namespace PhysicsCollisionStart;
        std::cout << "HandlePhysicsCollisionStart" << std::endl;
        assert(eventType == E_PHYSICSCOLLISIONSTART);

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

        // bodies
        auto bodya = static_cast<RigidBody*>(eventData[P_BODYA].GetPtr());
        std::cout << "body a mass " << bodya->GetMass() << std::endl;
        auto bodyb = static_cast<RigidBody*>(eventData[P_BODYB].GetPtr());
        std::cout << "body b mass " << bodyb->GetMass() << std::endl;

        // trigger
        auto trigger = static_cast<Node*>(eventData[P_TRIGGER].GetPtr());
        std::cout << "trigger " << trigger << std::endl;

        // contact
        MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
        while (!contacts.IsEof()) {
            auto contactPosition = contacts.ReadVector3();
            auto contactNormal = contacts.ReadVector3();
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
        scene->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
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
