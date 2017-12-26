/*
Expected outcome:

- press ESC: quit
- release space: stdout shows a message

HandleKeyDown and HandleKeyUp are mostly useless I think, Input->GetKeyDown is the more reliable method.
*/

#include "common.hpp"

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartExtra() override {
        SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Main, HandleKeyUp));
    }
private:
    void HandleKeyDownExtra(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        auto key = eventData[P_KEY].GetInt();
        if (key == KEY_SPACE) {
            std::cout << "space down" << std::endl;
        }
    }
    void HandleKeyUp(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyUp;
        auto key = eventData[P_KEY].GetInt();
        if (key == KEY_SPACE) {
            std::cout << "space up" << std::endl;
        }
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
