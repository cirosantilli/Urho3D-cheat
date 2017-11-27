/*
Expected outcome:

- press ESC: quit
- release space: stdout shows a message
*/

#include <iostream>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>

using namespace Urho3D;

class Main : public Application {
    URHO3D_OBJECT(Main, Application);
public:
    Main(Context* context) : Application(context) {
    }
    virtual void Setup() override {
        this->engineParameters_[EP_FULL_SCREEN] = false;
        this->engineParameters_[EP_WINDOW_TITLE] = __FILE__ " SPACE and ESC";
        GetSubsystem<Input>()->SetMouseVisible(true);
    }
    void Start() {
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));
        SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Main, HandleKeyUp));
    }
    void Stop() {}
private:
    void HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        auto key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            engine_->Exit();
        }
    }
    void HandleKeyUp(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyUp;
        auto key = eventData[P_KEY].GetInt();
        if (key == KEY_SPACE) {
            std::cout << "space down" << std::endl;
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
