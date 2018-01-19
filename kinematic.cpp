/*
Expected outcome: ball goes up, due to the velocity applied, ignoring the force of gravity.
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->GetWindowWidth()));
        auto node = this->scene->CreateChild("Ball");
        node->SetPosition(Vector2(
            this->GetWindowWidth() / 2.0f,
            this->GetWindowHeight() / 2.0f
        ));
        auto body = node->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_KINEMATIC);
        //body->SetBodyType(BT_DYNAMIC);
        body->SetLinearVelocity(Vector2::UP * this->GetWindowWidth() / 4.0f);
        auto shape = node->CreateComponent<CollisionCircle2D>();
        shape->SetRadius(this->GetWindowWidth() / 20.0f);
        shape->SetDensity(1.0f);
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
