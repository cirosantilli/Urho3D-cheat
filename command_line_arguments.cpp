/*
Expected outcome: show the sum of the -zx and -zy arguments to the screen.

Sample call:

    ./progname -zx 1 -zy 2

would show 3 on the screen.

The engine highjacks a ton of CLI arguments at Engine.cpp,
so we prefix all our argments with "z" to help prevent clashes.
*/

#include "common.hpp"

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
    virtual void StartOnce() override {
        unsigned long x = 0, y = 1;
        auto args = GetArguments();
        decltype(args.Size()) i = 0;
        while (i < args.Size()) {
            auto& arg = args[i];
            if (arg == "-zx") {
                ++i;
                x = std::stoul(args[i].CString());
            } else if (arg == "-zy") {
                ++i;
                y = std::stoul(args[i].CString());
            }
            ++i;
        }
        auto text = this->GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
        text->SetFont(this->GetSubsystem<ResourceCache>()->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
        text->SetHorizontalAlignment(HA_CENTER);
        text->SetVerticalAlignment(VA_CENTER);
        text->SetText(std::to_string(x + y).c_str());
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
