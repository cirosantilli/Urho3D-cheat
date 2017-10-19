#include <Urho3D/Engine/Application.h>

using namespace Urho3D;

class Main : public Application {
    URHO3D_OBJECT(Main, Application);
public:
    Main(Context* context) : Application(context) {}
    virtual void Setup() override {}
    void Start() {}
    void Stop() {}
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
