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

        Node *leftCalfNode = leftCalfNode;
        leftCalfNode = groundNode->Clone();
        leftCalfNode->SetName("LeftCalf");
        leftCalfNode->SetPosition(Vector2(this->windowWidth / 2.0f, this->calfLength / 2.0f + this->thickness));
        {
            auto body = leftCalfNode->GetComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            auto shape = leftCalfNode->GetComponent<CollisionBox2D>();
            shape->SetSize(Vector2(thickness, calfLength));
            shape->SetDensity(this->density);
            shape->SetFriction(1000.0f);
            shape->SetRestitution(0.0);
        }

        Node *rightCalfNode = rightCalfNode;
        rightCalfNode = leftCalfNode->Clone();
        rightCalfNode->SetName("RightCalf");
        rightCalfNode->Translate(Vector2::RIGHT * 2.0f * (this->thighLength + this->thickness));

        Node *leftThighNode = leftCalfNode->Clone();
        leftThighNode->SetName("LeftleftThigh");
        leftThighNode->Translate(Vector2(
            this->thighLength / 2.0f + this->thickness / 2.0f,
            this->calfLength / 2.0f + this->thickness / 2.0f
        ));
        leftThighNode->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
        auto leftThighBody = leftThighNode->GetComponent<RigidBody2D>();

        Node *rightThighNode = leftCalfNode->Clone();
        rightThighNode->SetName("RightThigh");
        rightThighNode->Translate(Vector2(
            1.5f * (this->thighLength + this->thickness),
            this->calfLength / 2.0f + this->thickness / 2.0f
        ));
        rightThighNode->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
        auto rightThighBody = rightThighNode->GetComponent<RigidBody2D>();

        Node *upperArmNode = leftCalfNode->Clone();
        upperArmNode->SetName("UpperArm");
        upperArmNode->Translate(Vector2(
            this->calfLength + this->thickness,
            this->thighLength + this->thickness
        ));
        auto upperArmBody = upperArmNode->GetComponent<RigidBody2D>();

        Node *foreArmNode = leftCalfNode->Clone();
        foreArmNode->SetName("ForeArm");
        foreArmNode->Translate(Vector2(
            this->calfLength + this->thickness,
            2.0f * (this->thighLength + this->thickness)
        ));
        auto foreArmBody = foreArmNode->GetComponent<RigidBody2D>();

        {
            auto& constraint = this->leftCalfLeftThighConstraint;
            constraint = leftCalfNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(leftThighBody);
            constraint->SetAnchor(
                leftCalfNode->GetPosition2D() +
                Vector2(0.0, this->calfLength / 2.0f + this->thickness / 2.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = this->rightCalfRightThighConstraint;
            constraint = rightCalfNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(rightThighBody);
            constraint->SetAnchor(
                rightCalfNode->GetPosition2D() +
                Vector2(0.0, this->calfLength / 2.0f + this->thickness / 2.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = this->leftThighRightThighConstraint;
            constraint = leftThighNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(rightThighBody);
            constraint->SetAnchor(
                leftThighNode->GetPosition2D() +
                Vector2(this->calfLength / 2.0f + this->thickness / 2.0f, 0.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = this->leftThighUpperArmConstraint;
            constraint = leftThighNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(upperArmBody);
            constraint->SetAnchor(
                leftThighNode->GetPosition2D() +
                Vector2(this->calfLength / 2.0f + this->thickness / 2.0f, 0.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = this->upperArmForeArmConstraint;
            constraint = upperArmNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(foreArmBody);
            constraint->SetAnchor(
                upperArmNode->GetPosition2D() +
                Vector2(0.0f, this->calfLength / 2.0f + this->thickness / 2.0f)
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
    static constexpr float calfLength = windowWidth / 10.0f;
    static constexpr float thighLength = calfLength;
    static constexpr float maxMotorTorque = 10.0f * density;
    static constexpr float motorSpeed = 10.0f;

    ConstraintRevolute2D
        *leftCalfLeftThighConstraint,
        *leftThighRightThighConstraint,
        *leftThighUpperArmConstraint,
        *rightCalfRightThighConstraint,
        *upperArmForeArmConstraint
    ;

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace Update;
        if (this->input->GetKeyDown(KEY_A)) {
            this->leftCalfLeftThighConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_D)) {
            this->leftCalfLeftThighConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->leftCalfLeftThighConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_W)) {
            this->leftThighRightThighConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_S)) {
            this->leftThighRightThighConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->rightCalfRightThighConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_J)) {
            this->rightCalfRightThighConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_L)) {
            this->rightCalfRightThighConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->rightCalfRightThighConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_I)) {
            this->leftThighUpperArmConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_K)) {
            this->leftThighUpperArmConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->leftThighUpperArmConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_Q)) {
            this->upperArmForeArmConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_E)) {
            this->upperArmForeArmConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->upperArmForeArmConstraint->SetMotorSpeed(0.0f);
        }
    }

};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
