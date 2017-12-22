/*
Biped robot with a picker arm, and some balls lying around in the scene.
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
        } {
            auto node = groundNode->Clone();
            node->SetName("TopWall");
            node->SetPosition(Vector2(this->windowWidth / 2.0f, this->windowHeight - (this->thickness / 2.0f)));
        } {
            auto node = groundNode->Clone();
            node->SetName("RightWall");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector2(this->windowWidth - (this->thickness / 2.0f), this->windowHeight / 2.0f));
        } {
            auto node = groundNode->Clone();
            node->SetName("LeftWall");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector2(this->thickness / 2.0f, this->windowHeight / 2.0f));
        }

        auto leftCalfNode = groundNode->Clone();
        leftCalfNode->SetName("LeftCalf");
        leftCalfNode->SetPosition(Vector2(this->windowWidth / 2.0f, this->calfLength / 2.0f + this->thickness));
        {
            auto body = leftCalfNode->GetComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            auto shape = leftCalfNode->GetComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->thickness, this->calfLength));
            shape->SetDensity(this->density);
            shape->SetFriction(this->friction);
            shape->SetRestitution(this->restitution);
        }

        auto rightCalfNode = leftCalfNode->Clone();
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

        auto rightThighNode = leftCalfNode->Clone();
        rightThighNode->SetName("RightThigh");
        rightThighNode->Translate(Vector2(
            1.5f * (this->thighLength + this->thickness),
            this->calfLength / 2.0f + this->thickness / 2.0f
        ));
        rightThighNode->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
        auto rightThighBody = rightThighNode->GetComponent<RigidBody2D>();

        auto upperArmNode = leftCalfNode->Clone();
        upperArmNode->SetName("UpperArm");
        upperArmNode->Translate(Vector2(
            this->calfLength + this->thickness,
            this->thighLength + this->thickness
        ));
        auto upperArmBody = upperArmNode->GetComponent<RigidBody2D>();

        auto foreArmNode = leftCalfNode->Clone();
        foreArmNode->SetName("ForeArm");
        foreArmNode->Translate(Vector2(
            this->calfLength + this->thickness,
            2.0f * (this->thighLength + this->thickness)
        ));
        auto foreArmBody = foreArmNode->GetComponent<RigidBody2D>();
        {
            auto shape = foreArmNode->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->handLength, this->thickness));
            shape->SetDensity(this->density);
            shape->SetFriction(this->friction);
            shape->SetRestitution(this->restitution);
            shape->SetCenter(Vector2(
                this->handLength * std::sin(this->handAngle),
                this->calfLength + this->handLength * std::cos(this->handAngle)
            ) / 2.0f);
            shape->SetAngle(this->handAngle);
        }
        {
            auto shape = foreArmNode->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->handLength, this->thickness));
            shape->SetDensity(this->density);
            shape->SetFriction(this->friction);
            shape->SetRestitution(this->restitution);
            shape->SetCenter(Vector2(
                this->handLength * std::sin(-this->handAngle),
                this->calfLength + this->handLength * std::cos(this->handAngle)
            ) / 2.0f);
            shape->SetAngle(-this->handAngle);
        }

        auto leftFingerNode = leftCalfNode->Clone();
        leftFingerNode->SetName("LeftFinger");
        leftFingerNode->SetPosition(
            foreArmNode->GetPosition2D() +
            Vector2(
                this->handLength * std::sin(-this->handAngle),
                this->calfLength + this->handLength * std::cos(this->handAngle)
            )
        );
        auto leftFingerBody = leftFingerNode->GetComponent<RigidBody2D>();
        {
            auto shape = leftFingerNode->GetComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->thickness, this->handLength));
        }

        auto rightFingerNode = leftFingerNode->Clone();
        rightFingerNode->SetName("RightFinger");
        rightFingerNode->SetPosition(
            foreArmNode->GetPosition2D() +
            Vector2(
                this->handLength * std::sin(this->handAngle),
                this->calfLength + this->handLength * std::cos(this->handAngle)
            )
        );
        auto rightFingerBody = rightFingerNode->GetComponent<RigidBody2D>();

        // Constraints.
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
        {
            auto& constraint = this->foreArmLeftFingerConstraint;
            constraint = foreArmNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(leftFingerBody);
            constraint->SetAnchor(
                leftFingerNode->GetPosition2D() -
                Vector2(0.0f, this->handLength)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = this->foreArmRightFingerConstraint;
            constraint = foreArmNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(rightFingerBody);
            constraint->SetAnchor(
                rightFingerNode->GetPosition2D() -
                Vector2(0.0f, this->handLength)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(this->maxMotorTorque);
            constraint->SetEnableMotor(true);
        }

        auto ballOnHandNode = this->scene->CreateChild("BallOnHand");
        {
            auto& node = ballOnHandNode;
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            node->SetPosition(foreArmNode->GetPosition2D() + Vector2::UP * this->calfLength * 1.5f);
            auto shape = node->CreateComponent<CollisionCircle2D>();
            shape->SetDensity(this->density);
            shape->SetFriction(this->friction);
            shape->SetRadius(this->thickness);
            shape->SetRestitution(this->restitution);
        }

        auto ballOnFloorNode = ballOnHandNode->Clone();
        ballOnFloorNode->SetName("BallOnFloor");
        ballOnFloorNode->SetPosition(leftCalfNode->GetPosition() + Vector2::LEFT * this->calfLength);
    }
private:
    static constexpr float density = 1.0f;
    static constexpr float groundWidth = windowWidth;
    static constexpr float thickness = windowWidth / 50.0f;
    static constexpr float calfLength = windowWidth / 10.0f;
    static constexpr float handLength = calfLength / 2.0f;
    static constexpr float handAngle = 45.0f;
    static constexpr float thighLength = calfLength;
    static constexpr float maxMotorTorque = 10.0f * density;
    static constexpr float motorSpeed = 10.0f;
    static constexpr float friction = 1000.0f;
    static constexpr float restitution = 0.0f;

    ConstraintRevolute2D
        *foreArmLeftFingerConstraint,
        *foreArmRightFingerConstraint,
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
        if (this->input->GetKeyDown(KEY_U)) {
            this->foreArmLeftFingerConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_O)) {
            this->foreArmLeftFingerConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->foreArmLeftFingerConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_Y)) {
            this->foreArmRightFingerConstraint->SetMotorSpeed(this->motorSpeed);
        } else if (this->input->GetKeyDown(KEY_H)) {
            this->foreArmRightFingerConstraint->SetMotorSpeed(-this->motorSpeed);
        } else {
            this->foreArmRightFingerConstraint->SetMotorSpeed(0.0f);
        }
    }

};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
