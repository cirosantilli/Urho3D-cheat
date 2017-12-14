/*
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->windowWidth));

        auto groundNode = this->scene->CreateChild("Ground");
        groundNode->SetPosition(Vector3(this->windowWidth / 2.0f, this->thickness / 2.0f, 0.0f));
        groundNode->CreateComponent<RigidBody2D>();
        {
            auto shape = groundNode->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->groundWidth, this->thickness));
            shape->SetDensity(this->density);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        }

        Node *leftThighNode = leftThighNode;
        leftThighNode = groundNode->Clone();
        leftThighNode->SetName("LeftThigh");
        leftThighNode->SetPosition(Vector2(this->windowWidth / 2.0f, this->thighLength / 2.0f + this->thickness));
        {
            auto body = leftThighNode->GetComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            auto shape = leftThighNode->GetComponent<CollisionBox2D>();
            shape->SetSize(Vector2(thickness, thighLength));
            shape->SetDensity(this->density);
            shape->SetFriction(1000.0f);
            shape->SetRestitution(0.0);
        }

        Node *rightThighNode = rightThighNode;
        rightThighNode = leftThighNode->Clone();
        rightThighNode->SetName("RightThigh");
        rightThighNode->Translate(Vector2::RIGHT * (this->hipLength + this->thickness));

        Node *hipNode = leftThighNode->Clone();
        hipNode->SetName("Hip");
        hipNode->Translate(Vector2(
            this->hipLength / 2.0f + this->thickness / 2.0f,
            this->thighLength / 2.0f + this->thickness / 2.0f)
        );
        hipNode->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
        auto hipBody = hipNode->GetComponent<RigidBody2D>();

        {
            auto& constraint = leftThighHipConstraint;
            constraint = leftThighNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(hipBody);
            constraint->SetAnchor(
                leftThighNode->GetPosition2D() +
                Vector2(0.0, this->thighLength / 2.0f + this->thickness / 2.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = rightThighHipConstraint;
            constraint = rightThighNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(hipBody);
            constraint->SetAnchor(
                rightThighNode->GetPosition2D()
                + Vector2(0.0, this->thighLength / 2.0f + this->thickness / 2.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
    }
private:
    static constexpr float density = 1.0f;
    static constexpr float groundWidth = windowWidth;
    static constexpr float thickness = windowWidth / 50.0f;
    static constexpr float thighLength = windowWidth / 10.0f;
    static constexpr float hipLength = 1.2f * thighLength;
    static constexpr float maxMotorTorque = 10.0f * density;
    static constexpr float motorSpeed = 10.0f;

    ConstraintRevolute2D *leftThighHipConstraint, *rightThighHipConstraint;

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace Update;
        if (this->input->GetKeyDown(KEY_A)) {
            this->leftThighHipConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_D)) {
            this->leftThighHipConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->leftThighHipConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_J)) {
            this->rightThighHipConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_L)) {
            this->rightThighHipConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->rightThighHipConstraint->SetMotorSpeed(0.0f);
        }
    }

};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
