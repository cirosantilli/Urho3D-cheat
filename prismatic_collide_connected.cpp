/*
https://stackoverflow.com/questions/47782981/why-dont-2d-bodies-with-a-constraintprismatic2d-collide-despite-setcollideconne
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->windowWidth));

        auto floorNode = this->scene->CreateChild("Floor");
        floorNode->SetPosition(Vector2(this->windowWidth / 2.0, this->windowWidth / 2.0 - this->floorWidth));
        auto floorBody = floorNode->CreateComponent<RigidBody2D>();
        auto shape = floorNode->CreateComponent<CollisionBox2D>();
        shape->SetSize(Vector2(floorLength, floorWidth));
        shape->SetFriction(1.0f);
        shape->SetRestitution(0.0);
        shape->SetDensity(1.0f);

        // Does not collide.
        {
            auto node = this->scene->CreateChild("Prismatic");
            node->SetPosition(Vector2(this->windowWidth / 4.0f, this->windowHeight * 3.0f / 4.0f));

            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);

            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.0);
            shape->SetSize(Vector2(this->windowWidth / 8.0, this->windowHeight / 8.0));

            auto constraint = node->CreateComponent<ConstraintPrismatic2D>();
            constraint->SetOtherBody(floorBody);
            constraint->SetAxis(Vector2(1.0f, -1.0f));
            constraint->SetAnchor(Vector2(this->windowWidth / 2.0, this->windowWidth / 2.0 - this->floorWidth));
            constraint->SetCollideConnected(true);
        }

        // Collides.
        {
            auto node = this->scene->CreateChild("Rope");
            node->SetPosition(Vector2(this->windowWidth * 3.0f / 4.0f, this->windowHeight * 0.75f));

            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);

            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.0);
            shape->SetSize(Vector2(this->windowWidth / 8.0, this->windowHeight / 8.0));

            auto constraint = node->CreateComponent<ConstraintRope2D>();
            constraint->SetOtherBody(floorBody);
            constraint->SetOwnerBodyAnchor(Vector2(0.0f, 0.0f));
            constraint->SetCollideConnected(true);
            constraint->SetMaxLength(this->windowWidth);
            constraint->SetCollideConnected(true);
        }
    }
private:
    static constexpr float floorLength = windowWidth;
    static constexpr float floorWidth = windowWidth / 20.0f;
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
