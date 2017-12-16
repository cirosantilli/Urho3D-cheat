/*
Expected outcome: two falling circles joined to one another.
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->windowWidth));

        // Ground
        {
            auto node = this->scene->CreateChild("Ground");
            node->SetPosition(Vector3(this->windowWidth / 2.0f, this->groundHeight / 2.0f, 0.0f));
            node->CreateComponent<RigidBody2D>();
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->groundWidth, this->groundHeight));
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        }

        // Compound body
        {
            auto node = this->scene->CreateChild("Balls");
            node->SetPosition(Vector3(windowWidth / 4.0f, this->windowHeight / 2.0f, 0.0f));
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            // Left shape.
            {
                auto shape = node->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(1.0f);
                shape->SetFriction(1.0f);
                shape->SetRadius(this->ballRadius * 2.0f);
                shape->SetRestitution(0.75f);
            }
            // Right shape.
            {
                auto shape = node->CreateComponent<CollisionCircle2D>();
                shape->SetCenter(Vector2(3.0f * this->ballRadius, 0.0f));
                shape->SetDensity(1.0f);
                shape->SetFriction(1.0f);
                shape->SetRadius(this->ballRadius);
                shape->SetRestitution(0.75f);
            }
        }
    }
private:
    static constexpr float groundWidth = windowWidth;
    static constexpr float groundHeight = windowWidth / 10.0f;
    static constexpr float ballRadius = windowWidth / 20.0f;
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
