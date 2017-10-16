# Urho3D Cheat

Urho3D extra examples and demos. Tested in Ubuntu 17.04.

Build Urho3D:

    cd Urho3D
    mkdir build
    cd build
    cmake -DURHO3D_LIB_TYPE=SHARED ..
    make -j`nproc`
    sudo make install

Build our examples:

    mkdir build
    cd build
    cmake ..
    make -j`nproc`
    ./bin/empty
    ./bin/input

Examples:

1. [empty.cpp](empty.cpp)
1. [input.cpp](input.cpp)
1. [collision.cpp](collision.cpp)
