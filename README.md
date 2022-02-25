# Urho3D Cheat

Urho3D extra examples and demos. Tested in Ubuntu 18.04.

More context at: https://cirosantilli.com/ciro-s-2d-reinforcement-learning-games

Video demo: https://www.youtube.com/watch?v=j_fl4xoGTKU

![](screenshot.png)

Build and run a demo natively:

```
git clone https://github.com/cirosantilli/Urho3D-cheat
cd Urho3D-cheat
git submodule update --init --depth 1
./run
```

The build is broken as of Ubuntu 21.10 with `multiple definition of WAYLAND_wl_proxy_marshal`, you could run it in Docker as mentioned at <https://stackoverflow.com/questions/16296753/can-you-run-gui-applications-in-a-linux-docker-container/71265066#71265066>:

```
sudo apt install tigervnc-viewer
sudo docker create --name rl2d -p 6080:80 -p 5900:5900 -w /root/rl2d -v "$(pwd)":/root/rl2d dorowu/ubuntu-desktop-lxde-vnc:bionic
sudo docker start rl2d
```

and on another terminal:

```
xtigervncviewer :5900
```

Then inside the VNC GUI guest, open a terminal with:

* Menu icon
* System Tools
* LXTerminal

and inside the guest terminal you can build and run normally:

```
cd ~/rl2
apt update
apt install -y cmake g++ libx11-dev libxext-dev libgl1-mesa-dev
./run
```

Note that this diretory of the host filesystem is shared with the Docker one, so you can edit files directly on your host after running:

```
sudo chown -R $USER:$USER .
```

Docker sets permission to `root:root` every time you run `docker start`. TODO find a solution: https://stackoverflow.com/questions/23439126/how-to-mount-a-host-directory-in-a-docker-container

The image inside VPN is a bit choppy when moving fast. But give me a break.

1.  [**Getting started**](getting-started.md)
1.  Minimal examples
    1.  [empty.cpp](empty.cpp)
    1.  [common_empty.cpp](empty.cpp)
    1.  [input.cpp](input.cpp)
    1.  [command_line_arguments.cpp](command_line_arguments.cpp)
    1.  [compound.cpp](compound.cpp)
    1.  [collision.cpp](collision.cpp)
    1.  [scale_sprite.cpp](scale_sprite.cpp)
    1.  [velocity_stop.cpp](velocity_stop.cpp)
    1.  [text.cpp](text.cpp)
    1.  [logic_component.cpp](logic_component.cpp)
    1.  [trigger](trigger.cpp)
1.  More interesting scenes
    1.  [pong.cpp](pong.cpp)
    1.  [biped.cpp](biped.cpp)
    1.  [food](food.cpp)
1.  3D
    1.  [collision3d.cpp](collision3d.cpp)
    1.  [compound3d.cpp](compound3d.cpp)
1.  [TODO](TODO.md)
    1.  [prismatic_collide_connected.cpp](prismatic_collide_connected.cpp)
    1.  [sprite_repeat](sprite_repeat.cpp)

Icons by <https://github.com/game-icons/icons>
