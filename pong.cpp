/*
Single player pong-like game, more like squash.

TODO:

- when ball hits left wall, give some visual / sound cue of damage, e.g. make background red. Possibly also give a good cue when it hits right wall.
- ball gets too fast when player hits it while moving up / down. How to prevent this?
- ball speed can get too vertical, and it takes forever to hit wall and come back
- ball speed can get too horizontal, and then it becomes too trivial. We need some random / adversarial aspect to make it more interesting.
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        // Application state.
        this->SetScore(0);

        // Scene
        {
            Node *bottomWallNode;
            RigidBody2D *wallBody;
            auto playerLength = this->GetWindowHeight() / 4.0f;
            auto wallLength = this->GetWindowWidth();
            auto wallWidth = this->GetWindowWidth() / 20.0f;
            {
                bottomWallNode = this->scene->CreateChild("BottomWall");
                bottomWallNode->SetPosition(Vector2(this->GetWindowWidth() / 2.0, wallWidth / 2.0f));
                wallBody = bottomWallNode->CreateComponent<RigidBody2D>();
                auto shape = bottomWallNode->CreateComponent<CollisionBox2D>();
                shape->SetSize(Vector2(wallLength, wallWidth));
                shape->SetRestitution(0.0);
            } {
                auto node = bottomWallNode->Clone();
                node->SetName("TopWall");
                node->SetPosition(Vector2(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() - (wallWidth / 2.0f)));
            } {
                auto& node = this->rightWallNode;
                node = bottomWallNode->Clone();
                node->SetName("RightWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector2(this->GetWindowWidth() - (wallWidth / 2.0f), this->GetWindowHeight() / 2.0f));
            } {
                auto& node = this->leftWallNode;
                node = bottomWallNode->Clone();
                node->SetName("LeftWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector2(-wallWidth / 2.0f, this->GetWindowHeight() / 2.0f));
            } {
                auto& node = this->playerNode;
                node = bottomWallNode->Clone();
                node->SetName("Player");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                // TODO The more elegant value of x is wallWidth / 2.0. But then we get stuck
                // to the left wall when the ball hits the player. Use collision filtering.
                node->SetPosition(Vector2(wallWidth, this->GetWindowHeight() / 2.0f));

                auto body = node->GetComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetFixedRotation(true);
                body->SetLinearDamping(5.0);

                // TODO can't collide with ground after setting prismatic constraint,
                // See prismatic_collide_connected.cpp
                auto bottomWallCloneNode = bottomWallNode->Clone();
                bottomWallCloneNode->SetName("Player");
                bottomWallCloneNode->SetPosition(Vector2(this->GetWindowHeight() / 2.0f, -this->GetWindowWidth()));

                auto constraint = node->CreateComponent<ConstraintPrismatic2D>();
                constraint->SetOtherBody(bottomWallCloneNode->GetComponent<RigidBody2D>());
                //constraint->SetOtherBody(wallBody);
                constraint->SetAxis(Vector2(0.0f, 1.0f));
                constraint->SetAnchor(Vector2(this->GetWindowWidth() / 2.0f, 0.0f));
                constraint->SetCollideConnected(true);

                // Main rectangle
                {
                    auto shape = node->GetComponent<CollisionBox2D>();
                    shape->SetDensity(this->playerDensity);
                    shape->SetFriction(this->playerFriction);
                    shape->SetRestitution(this->playerRestitution);
                    shape->SetSize(Vector2(playerLength, wallWidth));
                }
                // Upper circle
                {
                    auto shape = node->CreateComponent<CollisionCircle2D>();
                    shape->SetDensity(0.0f);
                    shape->SetFriction(this->playerFriction);
                    shape->SetRestitution(this->playerRestitution);
                    shape->SetCenter(Vector2(playerLength / 2.0f, 0.0f));
                    shape->SetRadius(wallWidth / 2.0f);
                }
                // Lower circle
                {
                    auto shape = node->CreateComponent<CollisionCircle2D>();
                    shape->SetDensity(0.0f);
                    shape->SetFriction(this->playerFriction);
                    shape->SetRestitution(this->playerRestitution);
                    shape->SetCenter(Vector2(-playerLength / 2.0f, 0.0f));
                    shape->SetRadius(wallWidth / 2.0f);
                }
            } {
                this->ballNode = this->scene->CreateChild("Ball");
                this->ballNode->SetPosition(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                auto body = this->ballNode->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetLinearVelocity(Vector2(2 * this->GetWindowWidth(), -this->GetWindowHeight() / 2.0f));
                auto shape = this->ballNode->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(this->ballDensity);
                shape->SetFriction(0.0f);
                shape->SetRadius(this->GetWindowWidth() / 20.0f);
                shape->SetRestitution(this->ballRestitution);
            }
        }
    }
    virtual void StartOnce() override {
        // Score
        this->text = this->GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
        this->text->SetFont(this->resourceCache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
        this->text->SetHorizontalAlignment(HA_CENTER);
        this->text->SetVerticalAlignment(VA_CENTER);
    }
private:
    Node *ballNode, *leftWallNode, *playerNode, *rightWallNode;
    Text *text;
    uint64_t score;

    static constexpr float ballRestitution = 1.0f;
    static constexpr float ballDensity = 1.0f;
    static constexpr float playerDensity = 10.0f * ballDensity;
    static constexpr float playerFriction = 1.0f;
    static constexpr float playerRestitution = 0.0f;

    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace PhysicsBeginContact2D;
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        Node *otherNode;
        bool isBall = false;
        if (nodea == this->ballNode) {
            otherNode = nodeb;
            isBall = true;
        }
        if (nodeb == this->ballNode) {
            otherNode = nodea;
            isBall = true;
        }
        if (isBall && otherNode == this->rightWallNode) {
            this->SetScore(this->score + 1);
        } else if (otherNode == this->leftWallNode) {
            this->SetScore(0);
        }
    }

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace Update;
        auto forceMagnitude = this->GetWindowWidth() * this->playerDensity * 10.0f;
        auto body = this->playerNode->GetComponent<RigidBody2D>();
        if (this->input->GetKeyDown(KEY_S)) {
            body->ApplyForceToCenter(Vector2::DOWN * forceMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_W)) {
            body->ApplyForceToCenter(Vector2::UP * forceMagnitude, true);
        }
    }

    void SetScore(uint64_t score) {
        this->score = score;
        this->text->SetText(std::to_string(this->score).c_str());
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
