# Urho3D Cheat

Urho3D extra examples and demos. Tested in Ubuntu 17.10.

Build and run a demo:

    ./run

Run another demo:

    cd build
    ./bin/biped

Modify an existing demo:

    vim biped.cpp
    cd bin
    make
    ./bin/biped

If you modify the common header and only want to rebuild one of the example:

    make biped
    ./bin/biped

Create a new example:

    vim my_new_example.cpp
    cd bin
    cmake ..
    make
    ./bin/my_new_example

Examples:

1.  [empty.cpp](empty.cpp)
1.  [common_empty.cpp](empty.cpp)
1.  [input.cpp](input.cpp)
1.  [command_line_arguments.cpp](command_line_arguments.cpp)
1.  [compound.cpp](compound.cpp)
1.  [collision.cpp](collision.cpp)
1.  [scale_sprite.cpp](scale_sprite.cpp)
1.  [velocity_stop.cpp](velocity_stop.cpp)
1.  [text.cpp](text.cpp)
1.  [pong.cpp](pong.cpp)
1.  [biped.cpp](biped.cpp)
1.  3D
    1.  [collision3d.cpp](collision3d.cpp)
    1.  [compound3d.cpp](compound3d.cpp)
1.  [TODO](TODO.md)
    1.  [food](food.cpp)
    1.  [prismatic_collide_connected.cpp](prismatic_collide_connected.cpp)
