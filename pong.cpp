/*
Single player pong-like game, more like squash.

TODO:

- make player edges rounded. Probably attach two colision circles to the edgest of the player.
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
        this->score = 0;
        this->text->SetText(std::to_string(this->score).c_str());

        // Scene
        {
            Node *bottomWallNode;
            RigidBody2D *wallBody;
            {
                bottomWallNode = this->scene->CreateChild("BottomWall");
                bottomWallNode->SetPosition(Vector2(this->windowWidth / 2.0, wallWidth / 2.0f));
                wallBody = bottomWallNode->CreateComponent<RigidBody2D>();
                auto box = bottomWallNode->CreateComponent<CollisionBox2D>();
                box->SetSize(Vector2(wallLength, wallWidth));
                box->SetRestitution(0.0);
            } {
                auto node = bottomWallNode->Clone();
                node->SetName("TopWall");
                node->SetPosition(Vector2(this->windowWidth / 2.0f, windowHeight - (wallWidth / 2.0f)));
            } {
                auto& node = this->rightWallNode;
                node = bottomWallNode->Clone();
                node->SetName("RightWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector2(this->windowWidth - (wallWidth / 2.0f), windowHeight / 2.0f));
            } {
                auto& node = this->leftWallNode;
                node = bottomWallNode->Clone();
                node->SetName("LeftWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector2(-wallWidth / 2.0f, windowHeight / 2.0f));
            } {
                auto& node = this->playerNode;
                node = bottomWallNode->Clone();
                node->SetName("Player");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                // TODO The more elegant value of x is wallWidth / 2.0. But then we get stuck
                // to the left wall when the ball hits the player. Use collision filtering.
                node->SetPosition(Vector2(wallWidth, windowHeight / 2.0f));

                auto body = node->GetComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetFixedRotation(true);
                body->SetLinearDamping(5.0);

                // TODO can't collide with ground after setting prismatic constraint,
                // despite `constraint->SetCollideConnected(true)`. Using a separate body as workaround for now,
                // will ask question later.
                auto bottomWallCloneNode = bottomWallNode->Clone();
                bottomWallCloneNode->SetName("Player");
                bottomWallCloneNode->SetPosition(Vector2(windowHeight / 2.0f, -windowWidth));

                auto constraint = node->CreateComponent<ConstraintPrismatic2D>();
                constraint->SetOtherBody(bottomWallCloneNode->GetComponent<RigidBody2D>());
                //constraint->SetOtherBody(wallBody);
                constraint->SetAxis(Vector2(0.0f, 1.0f));
                constraint->SetAnchor(Vector2(this->windowWidth / 2.0f, 0.0f));
                constraint->SetCollideConnected(true);

                auto shape = node->GetComponent<CollisionBox2D>();
                shape->SetDensity(this->playerDensity);
                shape->SetFriction(1.0f);
                shape->SetRestitution(0.0);
                shape->SetSize(Vector2(playerLength, wallWidth));
            } {
                this->ballNode = this->scene->CreateChild("Ball");
                this->ballNode->SetPosition(Vector2(this->windowWidth / 4.0f, windowHeight / 2.0f));
                auto body = this->ballNode->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetLinearVelocity(Vector2(2 * this->windowWidth, -windowHeight / 2.0f));
                auto shape = this->ballNode->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(this->ballDensity);
                shape->SetFriction(0.0f);
                shape->SetRadius(ballRadius);
                shape->SetRestitution(ballRestitution);
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
    void Stop() override {}
private:
    Node *ballNode, *leftWallNode, *playerNode, *rightWallNode;
    Text *text;
    uint64_t score;

    static constexpr float ballRadius = windowWidth / 20.0f;
    static constexpr float ballRestitution = 1.0f;
    static constexpr float ballDensity = 1.0f;
    static constexpr float playerDensity = 10.0f * ballDensity;
    static constexpr float playerLength = windowHeight / 4.0f;
    static constexpr float wallLength = windowWidth;
    static constexpr float wallWidth = windowWidth / 20.0f;

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
        if (otherNode == this->rightWallNode) {
            this->score++;
        } else if (otherNode == this->leftWallNode) {
            this->score = 0;
        }
        this->text->SetText(std::to_string(this->score).c_str());
    }

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace Update;
        auto forceMagnitude = this->windowWidth * this->playerDensity * 10.0f;
        auto body = this->playerNode->GetComponent<RigidBody2D>();
        if (this->input->GetKeyDown(KEY_S)) {
            body->ApplyForceToCenter(Vector2::DOWN * forceMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_W)) {
            body->ApplyForceToCenter(Vector2::UP * forceMagnitude, true);
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
