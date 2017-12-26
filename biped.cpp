/*
Biped robot with a picker arm, and some balls lying around in the scene.
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->GetWindowWidth()));
        auto density = 1.0f;
        auto groundWidth = this->GetWindowWidth();
        auto thickness = this->GetWindowWidth() / 50.0f;
        auto calfLength = this->GetWindowWidth() / 10.0f;
        auto handLength = calfLength / 2.0f;
        auto handAngle = 45.0f;
        auto thighLength = calfLength;
        auto maxMotorTorque = 10.0f * density;
        auto friction = 1000.0f;
        auto restitution = 0.0f;


        auto groundNode = this->scene->CreateChild("Ground");
        groundNode->SetPosition(Vector3(this->GetWindowWidth() / 2.0f, thickness / 2.0f, 0.0f));
        groundNode->CreateComponent<RigidBody2D>();
        {
            auto shape = groundNode->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(groundWidth, thickness));
            shape->SetDensity(density);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        } {
            auto node = groundNode->Clone();
            node->SetName("TopWall");
            node->SetPosition(Vector2(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() - (thickness / 2.0f)));
        } {
            auto node = groundNode->Clone();
            node->SetName("RightWall");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector2(this->GetWindowWidth() - (thickness / 2.0f), this->GetWindowHeight() / 2.0f));
        } {
            auto node = groundNode->Clone();
            node->SetName("LeftWall");
            node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
            node->SetPosition(Vector2(thickness / 2.0f, this->GetWindowHeight() / 2.0f));
        }

        auto leftCalfNode = groundNode->Clone();
        leftCalfNode->SetName("LeftCalf");
        leftCalfNode->SetPosition(Vector2(this->GetWindowWidth() / 2.0f, calfLength / 2.0f + thickness));
        {
            auto body = leftCalfNode->GetComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            auto shape = leftCalfNode->GetComponent<CollisionBox2D>();
            shape->SetSize(Vector2(thickness, calfLength));
            shape->SetDensity(density);
            shape->SetFriction(friction);
            shape->SetRestitution(restitution);
        }

        auto rightCalfNode = leftCalfNode->Clone();
        rightCalfNode->SetName("RightCalf");
        rightCalfNode->Translate(Vector2::RIGHT * 2.0f * (thighLength + thickness));

        Node *leftThighNode = leftCalfNode->Clone();
        leftThighNode->SetName("LeftleftThigh");
        leftThighNode->Translate(Vector2(
            thighLength / 2.0f + thickness / 2.0f,
            calfLength / 2.0f + thickness / 2.0f
        ));
        leftThighNode->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
        auto leftThighBody = leftThighNode->GetComponent<RigidBody2D>();

        auto rightThighNode = leftCalfNode->Clone();
        rightThighNode->SetName("RightThigh");
        rightThighNode->Translate(Vector2(
            1.5f * (thighLength + thickness),
            calfLength / 2.0f + thickness / 2.0f
        ));
        rightThighNode->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
        auto rightThighBody = rightThighNode->GetComponent<RigidBody2D>();

        auto upperArmNode = leftCalfNode->Clone();
        upperArmNode->SetName("UpperArm");
        upperArmNode->Translate(Vector2(
            calfLength + thickness,
            thighLength + thickness
        ));
        auto upperArmBody = upperArmNode->GetComponent<RigidBody2D>();

        auto foreArmNode = leftCalfNode->Clone();
        foreArmNode->SetName("ForeArm");
        foreArmNode->Translate(Vector2(
            calfLength + thickness,
            2.0f * (thighLength + thickness)
        ));
        auto foreArmBody = foreArmNode->GetComponent<RigidBody2D>();
        {
            auto shape = foreArmNode->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(handLength, thickness));
            shape->SetDensity(density);
            shape->SetFriction(friction);
            shape->SetRestitution(restitution);
            shape->SetCenter(Vector2(
                handLength * std::sin(handAngle),
                calfLength + handLength * std::cos(handAngle)
            ) / 2.0f);
            shape->SetAngle(handAngle);
        }
        {
            auto shape = foreArmNode->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(handLength, thickness));
            shape->SetDensity(density);
            shape->SetFriction(friction);
            shape->SetRestitution(restitution);
            shape->SetCenter(Vector2(
                handLength * std::sin(-handAngle),
                calfLength + handLength * std::cos(handAngle)
            ) / 2.0f);
            shape->SetAngle(-handAngle);
        }

        auto leftFingerNode = leftCalfNode->Clone();
        leftFingerNode->SetName("LeftFinger");
        leftFingerNode->SetPosition(
            foreArmNode->GetPosition2D() +
            Vector2(
                handLength * std::sin(-handAngle),
                calfLength + handLength * std::cos(handAngle)
            )
        );
        auto leftFingerBody = leftFingerNode->GetComponent<RigidBody2D>();
        {
            auto shape = leftFingerNode->GetComponent<CollisionBox2D>();
            shape->SetSize(Vector2(thickness, handLength));
        }

        auto rightFingerNode = leftFingerNode->Clone();
        rightFingerNode->SetName("RightFinger");
        rightFingerNode->SetPosition(
            foreArmNode->GetPosition2D() +
            Vector2(
                handLength * std::sin(handAngle),
                calfLength + handLength * std::cos(handAngle)
            )
        );
        auto rightFingerBody = rightFingerNode->GetComponent<RigidBody2D>();

        // Constraints.
        {
            auto& constraint = leftCalfLeftThighConstraint;
            constraint = leftCalfNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(leftThighBody);
            constraint->SetAnchor(
                leftCalfNode->GetPosition2D() +
                Vector2(0.0, calfLength / 2.0f + thickness / 2.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = rightCalfRightThighConstraint;
            constraint = rightCalfNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(rightThighBody);
            constraint->SetAnchor(
                rightCalfNode->GetPosition2D() +
                Vector2(0.0, calfLength / 2.0f + thickness / 2.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = leftThighRightThighConstraint;
            constraint = leftThighNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(rightThighBody);
            constraint->SetAnchor(
                leftThighNode->GetPosition2D() +
                Vector2(calfLength / 2.0f + thickness / 2.0f, 0.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = leftThighUpperArmConstraint;
            constraint = leftThighNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(upperArmBody);
            constraint->SetAnchor(
                leftThighNode->GetPosition2D() +
                Vector2(calfLength / 2.0f + thickness / 2.0f, 0.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = upperArmForeArmConstraint;
            constraint = upperArmNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(foreArmBody);
            constraint->SetAnchor(
                upperArmNode->GetPosition2D() +
                Vector2(0.0f, calfLength / 2.0f + thickness / 2.0f)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = foreArmLeftFingerConstraint;
            constraint = foreArmNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(leftFingerBody);
            constraint->SetAnchor(
                leftFingerNode->GetPosition2D() -
                Vector2(0.0f, handLength)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(maxMotorTorque);
            constraint->SetEnableMotor(true);
        }
        {
            auto& constraint = foreArmRightFingerConstraint;
            constraint = foreArmNode->CreateComponent<ConstraintRevolute2D>();
            constraint->SetOtherBody(rightFingerBody);
            constraint->SetAnchor(
                rightFingerNode->GetPosition2D() -
                Vector2(0.0f, handLength)
            );
            constraint->SetCollideConnected(true);
            constraint->SetMaxMotorTorque(maxMotorTorque);
            constraint->SetEnableMotor(true);
        }

        auto ballOnHandNode = this->scene->CreateChild("BallOnHand");
        {
            auto& node = ballOnHandNode;
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_DYNAMIC);
            node->SetPosition(foreArmNode->GetPosition2D() + Vector2::UP * calfLength * 1.5f);
            auto shape = node->CreateComponent<CollisionCircle2D>();
            shape->SetDensity(density);
            shape->SetFriction(friction);
            shape->SetRadius(thickness);
            shape->SetRestitution(restitution);
        }

        auto ballOnFloorNode = ballOnHandNode->Clone();
        ballOnFloorNode->SetName("BallOnFloor");
        ballOnFloorNode->SetPosition(leftCalfNode->GetPosition() + Vector2::LEFT * calfLength);
    }
private:
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
        auto motorSpeed = 10.0f;
        if (this->input->GetKeyDown(KEY_A)) {
            this->leftCalfLeftThighConstraint->SetMotorSpeed(motorSpeed);
        } else if (this->input->GetKeyDown(KEY_D)) {
            this->leftCalfLeftThighConstraint->SetMotorSpeed(-motorSpeed);
        } else {
            this->leftCalfLeftThighConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_W)) {
            this->leftThighRightThighConstraint->SetMotorSpeed(motorSpeed);
        } else if (this->input->GetKeyDown(KEY_S)) {
            this->leftThighRightThighConstraint->SetMotorSpeed(-motorSpeed);
        } else {
            this->rightCalfRightThighConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_J)) {
            this->rightCalfRightThighConstraint->SetMotorSpeed(motorSpeed);
        } else if (this->input->GetKeyDown(KEY_L)) {
            this->rightCalfRightThighConstraint->SetMotorSpeed(-motorSpeed);
        } else {
            this->rightCalfRightThighConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_I)) {
            this->leftThighUpperArmConstraint->SetMotorSpeed(motorSpeed);
        } else if (this->input->GetKeyDown(KEY_K)) {
            this->leftThighUpperArmConstraint->SetMotorSpeed(-motorSpeed);
        } else {
            this->leftThighUpperArmConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_Q)) {
            this->upperArmForeArmConstraint->SetMotorSpeed(motorSpeed);
        } else if (this->input->GetKeyDown(KEY_E)) {
            this->upperArmForeArmConstraint->SetMotorSpeed(-motorSpeed);
        } else {
            this->upperArmForeArmConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_U)) {
            this->foreArmLeftFingerConstraint->SetMotorSpeed(motorSpeed);
        } else if (this->input->GetKeyDown(KEY_O)) {
            this->foreArmLeftFingerConstraint->SetMotorSpeed(-motorSpeed);
        } else {
            this->foreArmLeftFingerConstraint->SetMotorSpeed(0.0f);
        }
        if (this->input->GetKeyDown(KEY_Y)) {
            this->foreArmRightFingerConstraint->SetMotorSpeed(motorSpeed);
        } else if (this->input->GetKeyDown(KEY_H)) {
            this->foreArmRightFingerConstraint->SetMotorSpeed(-motorSpeed);
        } else {
            this->foreArmRightFingerConstraint->SetMotorSpeed(0.0f);
        }
    }

};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
