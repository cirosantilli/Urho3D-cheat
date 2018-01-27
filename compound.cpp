/*
Expected outcome: two falling circles joined to one another.
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->GetWindowWidth()));
        auto ballRadius = this->GetWindowWidth() / 20.0f;
        auto groundHeight = this->GetWindowWidth() / 10.0f;
        auto groundWidth = this->GetWindowWidth();

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

        // Compound body
        {
            auto node = this->scene->CreateChild("Balls");
            node->SetPosition2D(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            // Left shape.
            {
                auto shape = node->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(1.0f);
                shape->SetFriction(1.0f);
                shape->SetRadius(ballRadius * 2.0f);
                shape->SetRestitution(0.75f);
            }
            // Right shape.
            {
                auto shape = node->CreateComponent<CollisionCircle2D>();
                shape->SetCenter(Vector2(3.0f * ballRadius, 0.0f));
                shape->SetDensity(1.0f);
                shape->SetFriction(1.0f);
                shape->SetRadius(ballRadius);
                shape->SetRestitution(0.75f);
            }
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
