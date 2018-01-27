/*
Circle moves up and down the screen.

Components are cool because they encapsulate functionality, which you can then attach to a node, and compose.
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {
        // Although this example appears to work without this,
        // others have failed. This is really needed.
        context->RegisterFactory<MaxDistComponent>();
    }
    virtual void StartExtra() override {
        auto ballNode = this->scene->CreateChild("Ball");
        ballNode->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() / 2.0f));
        auto maxDistLogicComponent = ballNode->CreateComponent<MaxDistComponent>();
        // There appears to be no way to have a constructor. Just use setters.
        maxDistLogicComponent->SetSpeed(Vector2::DOWN * this->GetWindowWidth() / 2.0f);
        maxDistLogicComponent->SetMaxDist(this->GetWindowWidth() / 2.0f);
        auto body = ballNode->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_KINEMATIC);
        auto shape = ballNode->CreateComponent<CollisionCircle2D>();
        shape->SetDensity(1.0f);
        shape->SetRadius(this->GetWindowWidth() / 20.0f);
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
