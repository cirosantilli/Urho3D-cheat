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
                node->SetRotation(Quaternion(90.0f));
                node->SetPosition(Vector2(this->windowWidth - (this->wallWidth / 2.0f), this->windowHeight / 2.0f));
            } {
                auto node = bottomWallNode->Clone();
                node->SetName("LeftWall");
                node->SetRotation(Quaternion(90.0f));
                node->SetPosition(Vector2(this->wallWidth / 2.0f, this->windowHeight / 2.0f));
            } {
                auto& node = this->playerNode;
                node = this->scene->CreateChild("Player");
                node->SetPosition(Vector2(this->windowWidth / 4.0f, windowHeight / 2.0f));
                auto& body = this->playerBody;
                body = node->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetLinearDamping(4.0);
                body->SetAngularDamping(4.0);
                body->SetBullet(true);
                auto shape = node->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(this->playerDensity);
                shape->SetFriction(0.0f);
                shape->SetRadius(this->playerRadius);
                shape->SetRestitution(this->playerRestitution);

                this->camera = node->CreateComponent<Camera>();
                this->camera->SetOrthoSize(this->windowWidth);
                this->camera->SetOrthographic(true);
                this->viewport = SharedPtr<Viewport>(new Viewport(this->context_, this->scene, this->camera));
                GetSubsystem<Renderer>()->SetViewport(0, this->viewport);
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
    struct ContactData {
        ContactData(Vector2 position, float impulse) :
            position(position), impulse(impulse) {}
        Vector2 position;
        float impulse;
    };

    static constexpr float playerDensity = 1.0f;
    static constexpr float playerFriction = 1.0f;
    static constexpr float playerLength = windowHeight / 4.0f;
    static constexpr float playerRadius = windowWidth / 20.0f;
    static constexpr float playerRestitution = 0.2f;
    static constexpr float wallLength = windowWidth;
    static constexpr float wallWidth = windowWidth / 20.0f;

    static void Rotate2D(Vector2& vec, float angle) {
        auto vec3 = Quaternion(angle) * vec;
        vec = Vector2(vec3.x_, vec3.y_);
    }

    RigidBody2D *playerBody;
    Node *appleNode, *playerNode;
    Text *text;
    uint64_t score;
    std::map<Node*,std::map<Node*,std::vector<ContactData>>> contactDataMap;

    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace PhysicsBeginContact2D;
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        Node *otherNode;
        bool player = false;
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

    virtual void HandlePhysicsUpdateContact2DExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace PhysicsUpdateContact2D;
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
        while (!contacts.IsEof()) {
            auto position = contacts.ReadVector2();
            auto normal = contacts.ReadVector2();
            auto distance = contacts.ReadFloat();
            auto impulse = contacts.ReadFloat();
            // TODO shared pointer here.
            this->contactDataMap[nodea][nodeb].push_back(ContactData(position, impulse));
            this->contactDataMap[nodeb][nodea].push_back(ContactData(position, impulse));
            if (false) {
                std::cout << "contact position " << position.ToString().CString() << std::endl;
                std::cout << "contact normal " << normal.ToString().CString() << std::endl;
                std::cout << "contact distance " << distance << std::endl;
                std::cout << "contact impulse " << impulse << std::endl;
            }
        }
    }

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace Update;

        auto playerPosition = this->playerNode->GetPosition2D();
        auto playerRotation = this->playerNode->GetRotation2D();
        Vector2 playerForwardDirection = Vector2::UP;
        this->Rotate2D(playerForwardDirection, playerRotation);

        // Camera sensor
        std::vector<Node*> results;
        auto nrays = 8u;
        auto angleStep = 360.0f / nrays;
        for (auto i = 0u; i < nrays; ++i) {
            auto angle = i * angleStep;
            auto direction = playerForwardDirection;
            this->Rotate2D(direction, angle);
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

        for (auto& keyVal : contactDataMap[this->playerNode]) {
            auto node = keyVal.first;
            auto contactDatas = keyVal.second;
            for (auto& contactData : contactDatas) {
                auto contactDirection = contactData.position - playerPosition;
                contactDirection.Normalize();
                auto contactAngle = Atan2(playerForwardDirection.y_, playerForwardDirection.x_) - Atan2(contactDirection.y_, contactDirection.x_);
                if (false) {
                    std::cout << "name: " << node->GetName().CString() << std::endl;
                    std::cout << "position: " << contactData.position.ToString().CString() << std::endl;
                    std::cout << "angle: " << contactAngle << std::endl;
                    std::cout << "impulse: " << contactData.impulse << std::endl;
                }
            }
            std::cout << std::endl;
        }

        // Act
        auto playerBody = this->playerNode->GetComponent<RigidBody2D>();

        // Linear movement
        {
            auto forceMagnitude = 4.0f * this->windowWidth * this->playerDensity;
            if (this->input->GetKeyDown(KEY_S)) {
                Vector2 direction = playerForwardDirection;
                this->Rotate2D(direction, 180.0);
                playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
            }
            if (this->input->GetKeyDown(KEY_W)) {
                Vector2 direction = playerForwardDirection;
                playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
            }
            if (this->input->GetKeyDown(KEY_A)) {
                Vector2 direction = playerForwardDirection;
                this->Rotate2D(direction, 90.0);
                playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
            }
            if (this->input->GetKeyDown(KEY_D)) {
                Vector2 direction = playerForwardDirection;
                this->Rotate2D(direction, -90.0);
                playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
            }
        }

        // Rotate
        if (this->input->GetKeyDown(KEY_Q)) {
            playerBody->ApplyTorque(this->playerDensity, true);
        }
        if (this->input->GetKeyDown(KEY_E)) {
            playerBody->ApplyTorque(-this->playerDensity, true);
        }

        contactDataMap.clear();
    }

    void SetScore(uint64_t score) {
        this->score = score;
        this->text->SetText(std::to_string(this->score).c_str());
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
