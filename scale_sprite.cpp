/*
Expected outcome: two falling boxes on a ground. Box sprites are correctly scaled to match the physical bodies.

- https://stackoverflow.com/questions/47488411/how-to-scale-a-sprite2d-in-urho3d-without-rescaling-the-entire-node
- https://discourse.urho3d.io/t/how-to-scale-a-sprite2d-in-urho3d-without-rescaling-the-entire-node/3785
*/

#include "common.hpp"

using namespace Urho3D;

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        this->physicsWorld->SetGravity(Vector2(0.0f, -this->windowWidth));

        // Ground
        {
            auto& node = this->groundNode;
            node = this->scene->CreateChild("Ground");
            node->SetPosition(Vector2(this->windowWidth / 2.0, this->groundHeight));
            auto body = node->CreateComponent<RigidBody2D>();
            body->SetBodyType(BT_STATIC);
            auto shape = node->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->groundWidth, this->groundHeight));
            shape->SetDensity(1.0f);
            shape->SetFriction(1.0f);
            shape->SetRestitution(0.75f);
        }

        // Boxes
        this->CreateBox(this->windowWidth / 2.0f, this->windowHeight / 2.0f, this->windowWidth /  5.0, this->windowWidth / 10.0);
        this->CreateBox(this->windowWidth / 2.0f, this->windowWidth / 4.0f, this->windowHeight / 10.0, this->windowWidth / 10.0);
    }
    virtual void StartOnce() override {
        this->boxSprite = this->resourceCache->GetResource<Sprite2D>("Urho2D/Box.png");
    }
private:
    static constexpr float groundWidth = windowWidth;
    static constexpr float groundHeight = windowWidth / 10.0f;

    Node *groundNode;
    Sprite2D *boxSprite;

    void CreateBox(float x, float y, float width, float height) {
        auto node = this->scene->CreateChild("Box");
        node->SetPosition(Vector3(x, y));
        node->SetRotation(Quaternion(0.0f, 0.0f, 30.0f));
        auto box = node->CreateComponent<CollisionBox2D>();
        box->SetDensity(1.0f);
        box->SetFriction(1.0f);
        box->SetRestitution(0.75f);
        box->SetSize(Vector2(width, height));
        auto body = node->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_DYNAMIC);
        auto staticSprite = node->CreateComponent<StaticSprite2D>();
        staticSprite->SetSprite(this->boxSprite);
        staticSprite->SetDrawRect(Rect(
            width / 2.0f,
            -height / 2.0f,
            -width / 2.0f,
            height / 2.0f
        ));
        staticSprite->SetUseDrawRect(true);
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
