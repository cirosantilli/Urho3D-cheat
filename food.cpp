/*
One agent eats apples and gets happy.
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
            {
                bottomWallNode = this->scene->CreateChild("BottomWall");
                bottomWallNode->SetPosition(Vector2(this->windowWidth / 2.0, wallWidth / 2.0f));
                wallBody = bottomWallNode->CreateComponent<RigidBody2D>();
                auto shape = bottomWallNode->CreateComponent<CollisionBox2D>();
                shape->SetSize(Vector2(wallLength, wallWidth));
                shape->SetRestitution(0.0);
            } {
                auto node = bottomWallNode->Clone();
                node->SetName("TopWall");
                node->SetPosition(Vector2(this->windowWidth / 2.0f, this->windowHeight - (this->wallWidth / 2.0f)));
            } {
                auto node = bottomWallNode->Clone();
                node->SetName("RightWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector2(this->windowWidth - (this->wallWidth / 2.0f), this->windowHeight / 2.0f));
            } {
                auto node = bottomWallNode->Clone();
                node->SetName("LeftWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector2(this->wallWidth / 2.0f, this->windowHeight / 2.0f));
            } {
                auto& node = this->playerNode;
                node = this->scene->CreateChild("Player");
                node->SetPosition(Vector2(this->windowWidth / 4.0f, windowHeight / 2.0f));
                auto& body = this->playerBody;
                body = node->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetLinearDamping(4.0);
                body->SetBullet(true);
                auto shape = node->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(this->playerDensity);
                shape->SetFriction(0.0f);
                shape->SetRadius(this->playerRadius);
                shape->SetRestitution(this->playerRestitution);
            } {
                auto& node = this->appleNode;
                node = this->scene->CreateChild("Apple");
                node->SetPosition(Vector2(this->windowWidth * 3.0f / 4.0f, windowHeight / 2.0f));
                auto body = node->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetBullet(true);
                auto shape = node->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(this->playerDensity);
                shape->SetFriction(0.0f);
                shape->SetRadius(this->playerRadius);
                shape->SetRestitution(this->playerRestitution);
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
    RigidBody2D *playerBody;
    Node *appleNode, *playerNode;
    Text *text;
    uint64_t score;

    static constexpr float playerDensity = 1.0f;
    static constexpr float playerFriction = 1.0f;
    static constexpr float playerLength = windowHeight / 4.0f;
    static constexpr float playerRadius = windowWidth / 20.0f;
    static constexpr float playerRestitution = 0.2f;
    static constexpr float wallLength = windowWidth;
    static constexpr float wallWidth = windowWidth / 20.0f;

    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace PhysicsBeginContact2D;
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        Node *otherNode;
        bool player;
        if (nodea == this->playerNode) {
            otherNode = nodeb;
            player = true;
        }
        if (nodeb == this->playerNode) {
            otherNode = nodea;
            player = true;
        }
        if (player && otherNode == this->appleNode) {
            this->SetScore(this->score + 1);
            //this->appleNode->Remove();
        }
    }

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace Update;

        // Camera sensor
        std::vector<Node*> results;
        auto nrays = 8u;
        auto angleStep = 360.0f / nrays;
        for (auto i = 0u; i < nrays; ++i) {
            auto angle = i * angleStep;
            auto direction3 = Quaternion(0.0f, 0.0f, angle) * Vector3::RIGHT;
            auto direction = Vector2(direction3.x_, direction3.y_);
            auto position = this->playerNode->GetPosition2D();
            auto startPoint = position + (this->playerRadius * direction);
            auto endPoint = position + (2.0f * this->windowWidth * direction);
            PhysicsRaycastResult2D result;
            this->physicsWorld->RaycastSingle(result, startPoint, endPoint);
            auto body = result.body_;
            if (nullptr == body) {
                results.push_back(nullptr);
            } else {
                results.push_back(body->GetNode());
            }
        }
        if (false) {
            for (const auto& result : results) {
                if (result == nullptr) {
                    std::cout << "nullptr" << std::endl;
                } else {
                    std::cout << result->GetName().CString() << std::endl;
                }
            }
            std::cout << std::endl;
        }

        Vector<std::tuple<Node*, Vector2, Vector<Vector2>>> contactList;
        this->playerBody->GetContactList(contactList);
        for (auto& contact : contactList) {
            Node *node;
            Vector2 normal;
            Vector<Vector2> positions;
            std::tie(node, normal, positions) = contact;
            if (positions.Size() > 0) {
                std::cout << this->steps << std::endl;
                std::cout << "name: " << node->GetName().CString() << std::endl;
                std::cout << "normal: " << normal.ToString().CString() << std::endl;
                for (auto& position : positions) {
                    std::cout << "position: " << position.ToString().CString() << std::endl;
                }
                std::cout << std::endl;
            }
        }

        // Act
        auto forceMagnitude = 4.0f * this->windowWidth * this->playerDensity;
        auto body = this->playerNode->GetComponent<RigidBody2D>();
        if (this->input->GetKeyDown(KEY_S)) {
            body->ApplyForceToCenter(Vector2::DOWN * forceMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_W)) {
            body->ApplyForceToCenter(Vector2::UP * forceMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_A)) {
            body->ApplyForceToCenter(Vector2::LEFT * forceMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_D)) {
            body->ApplyForceToCenter(Vector2::RIGHT * forceMagnitude, true);
        }
    }

    void SetScore(uint64_t score) {
        this->score = score;
        this->text->SetText(std::to_string(this->score).c_str());
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
