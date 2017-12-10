/*
Expected outcome: two falling balls. When either hits the ground, print a message to stdout.

https://stackoverflow.com/questions/47505166/how-to-detect-collisions-in-urho3d-in-a-2d-world
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        // Events
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->windowWidth));

        // Ground
        {
            auto& node = this->groundNode;
            node = this->scene->CreateChild("Ground");
            node->SetPosition(Vector3(this->windowWidth / 2.0f, this->groundHeight / 2.0f, 0.0f));
            node->CreateComponent<RigidBody2D>();
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->groundWidth, this->groundHeight));
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        }

        // Left ball
        {
            this->leftBallNode = this->scene->CreateChild("LeftBall");
            this->leftBallNode->SetPosition(Vector3(windowWidth / 4.0f, this->windowHeight / 2.0f, 0.0f));
            auto body = this->leftBallNode->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            auto shape = this->leftBallNode->CreateComponent<CollisionCircle2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRadius(this->ballRadius);
            shape->SetRestitution(0.75f);
        }

        // Right ball
        {
            auto& node = this->rightBallNode;
            node = this->leftBallNode->Clone();
            node->SetName("RightBall");
            node->SetPosition(Vector3(windowWidth * (3.0f / 4.0f), this->windowHeight * (3.0f / 4.0f), 0.0f));
        }
    }
private:
    static constexpr float groundWidth = windowWidth;
    static constexpr float groundHeight =windowWidth / 10.0f;
    static constexpr float ballRadius = windowWidth / 20.0f;

    Node *leftBallNode, *groundNode, *rightBallNode;

    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) override {
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
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
