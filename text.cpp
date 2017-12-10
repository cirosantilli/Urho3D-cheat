/*
Expected outcome: how text "hello" on center of screen.

It if fixed on the HUD, and does not move even if we move the camera around.
*/

#include "common.hpp"

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartOnce() override {
        auto text = this->GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
        text->SetFont(this->GetSubsystem<ResourceCache>()->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
        text->SetHorizontalAlignment(HA_CENTER);
        text->SetVerticalAlignment(VA_CENTER);
        text->SetText("hello");
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
