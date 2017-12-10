/*
Expected outcome: open a black fullscreen window.

To exit: alt + tab, then Ctrl + C on terminal.
*/

#include <Urho3D/Engine/Application.h>

using namespace Urho3D;

class Main : public Application {
    URHO3D_OBJECT(Main, Application);
public:
    Main(Context* context) : Application(context) {}
    virtual void Setup() override {}
    virtual void Start() override {}
    virtual void Stop() override {}
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
