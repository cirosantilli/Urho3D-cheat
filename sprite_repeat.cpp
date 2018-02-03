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
        auto groundWidth = this->GetWindowWidth();
        auto groundHeight = this->GetWindowWidth() / 10.0f;
        auto width = this->GetWindowWidth() / 2.0f;
        auto height = this->GetWindowHeight() / 4.0f;
        auto node = this->scene->CreateChild("Box");
        node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() / 2.0f));
        auto box = node->CreateComponent<CollisionBox2D>();
        box->SetSize(Vector2(width, height));
		auto body = node->CreateComponent<RigidBody2D>();
        auto boxSprite = this->resourceCache->GetResource<Sprite2D>("Urho2D/Box.png");
        //boxSprite->SetRectangle(IntRect(0, 0, 100, 100));
        auto staticSprite = node->CreateComponent<StaticSprite2D>();
		staticSprite->SetCustomMaterial(this->resourceCache->GetResource<Material>("Materials/StoneTiled.xml"));
		//staticSprite->SetSprite(boxSprite);
        //staticSprite->SetDrawRect(Rect(
            //width / 2.0f,
            //-height / 2.0f,
            //-width / 2.0f,
            //height / 2.0f
        //));
        //staticSprite->SetUseDrawRect(true);
		//staticSprite->SetTextureRect(Rect(
			//width / 2.0f,
			//-height / 2.0f,
			//-width / 2.0f,
			//height / 2.0f
		//));
		//staticSprite->SetUseTextureRect(true);
    }
private:
    Node *groundNode;
    Sprite2D *boxSprite;
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
