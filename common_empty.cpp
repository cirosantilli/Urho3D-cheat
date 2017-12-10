/*
Illustrate common.hpp.
*/

#include "common.hpp"

class Main : public Common {
public:
    Main(Context* context) : Common(context) {}
};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
