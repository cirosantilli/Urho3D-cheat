/*
Events are triggered whenever the balls touch the trigger body.

But ball movement is not affected at all by it, they just go through.

Uses Box2D sensor body.
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->GetWindowWidth()));
        auto groundWidth = this->GetWindowWidth();
        auto groundHeight = this->GetWindowHeight()/ 10.0f;
        auto ballRadius = this->GetWindowWidth() / 20.0f;
        // Ground
        {
            auto node = this->scene->CreateChild("Ground");
            node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0f, groundHeight / 2.0f));
            node->CreateComponent<RigidBody2D>();
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(groundWidth, groundHeight));
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        }
        // Left ball
        Node *leftBallNode;
        {
            leftBallNode = this->scene->CreateChild("LeftBall");
            leftBallNode->SetPosition2D(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
            auto body = leftBallNode->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            auto shape = leftBallNode->CreateComponent<CollisionCircle2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRadius(ballRadius);
            shape->SetRestitution(0.75f);
        }
        // Right ball
        {
            auto node = leftBallNode->Clone();
            node->SetName("RightBall");
            node->SetPosition2D(Vector2(this->GetWindowWidth() * (3.0f / 4.0f), this->GetWindowHeight() * (3.0f / 4.0f)));
        }
        // Trigger body.
        {
            auto node = this->scene->CreateChild("Trigger");
            node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() / 2.0f));
            node->SetRotation(Quaternion(15.0f));
            node->CreateComponent<RigidBody2D>();
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(groundWidth, groundHeight));
            shape->SetTrigger(true);
            node->SubscribeToEvent(node, E_NODEBEGINCONTACT2D, [&](StringHash eventType, VariantMap& eventData) {
                std::cout
                    << "E_NODEBEGINCONTACT2D "
                    << static_cast<Node*>(eventData[NodeBeginContact2D::P_OTHERNODE].GetPtr())->GetName().CString()
                    << " "
                    << this->steps
                    << std::endl
                ;
            });
            node->SubscribeToEvent(node, E_NODEENDCONTACT2D, [&](StringHash eventType, VariantMap& eventData) {
                std::cout
                    << "E_NODEENDCONTACT2D "
                    << static_cast<Node*>(eventData[NodeEndContact2D::P_OTHERNODE].GetPtr())->GetName().CString()
                    << " "
                    << this->steps
                    << std::endl
                ;
            });
            // Never called. TODO why.
            node->SubscribeToEvent(node, E_NODEUPDATECONTACT2D, [&](StringHash eventType, VariantMap& eventData) {
                std::cout
                    << "E_NODEUPDATECONTACT2D "
                    << static_cast<Node*>(eventData[NodeUpdateContact2D::P_OTHERNODE].GetPtr())->GetName().CString()
                    << " "
                    << this->steps
                    << std::endl
                ;
            });
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
