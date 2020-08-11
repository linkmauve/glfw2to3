# GLFW 2to3

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

But sadly, many games were using its older 2.x version, which was a lot more
limited and didn’t support as many platforms.  This project makes it possible
to translate those older games into using GLFW 3.x instead, for instance for
Wayland support.


## Compiling GLFW 2to3

Install [meson](https://mesonbuild.com/) and [glfw](https://www.glfw.org/), and
run:
```shell
% meson build
% ninja -C build
```

You can then replace your game’s `libglfw.so.2` with the one you just built in
the `build/` directory.  Enjoy! :)
