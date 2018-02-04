# Getting started

Run another demo:

    cd build
    ./bin/biped

Modify an existing demo:

    vim biped.cpp
    cd build
    make
    ./bin/biped

If you modify the common header and only want to rebuild one of the example:

    make biped
    ./bin/biped

Create a new example:

    vim my_new_example.cpp
    ./build-demos

Hack up Urho and test it out on a demo:

    ./build-urho
    ./build-demos
