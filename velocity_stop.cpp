/*
We have to hack b2Settings::b2_velocityThreshold in the Box2D source to something below 1.0
for the 1.0 velocity to bounce back. 1.01 bounces back fine.

- https://gamedev.stackexchange.com/questions/84737/why-do-my-box2d-bodies-occasionally-get-stuck-and-separate-forcibly/151878#151878
- https://stackoverflow.com/questions/5381399/how-can-i-prevent-a-ball-from-sticking-to-walls-in-box2d
*/

#include "common.hpp"

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        auto velocity = 1.00f;
        //auto velocity = 1.01f;
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
            shape->SetDensity(1.0);
            shape->SetFriction(1.0f);
            shape->SetRestitution(1.0);
        }

        // Ball
        {
            auto node = this->scene->CreateChild("Ball");
            node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() / 2.0f));
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
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
