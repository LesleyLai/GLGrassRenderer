# OpenGL Grass Renderer
This project is a C++/OpenGL implementation of [Responsive Real-Time Grass Rendering for General 3D Scenes](https://www.cg.tuwien.ac.at/research/publications/2017/JAHRMANN-2017-RRTG/JAHRMANN-2017-RRTG-draft.pdf). The project uses Bezier curves to represent individual grass blades. It uses compute shader to perform force simulation in Euler's method and perform various culling methods. Afterward the buffer of curves is passed to a tessellation shader to dynamically generate triangle geometry for grass blades.

![demo image](image.gif)

Tested on:
- Windows 10, i7-8650U @ 1.90GHz, GTX 1050 (laptop)
- Same laptop with Intel UHD Graphics 620 Integrated GPU **does not work** currently, need to figure out why
- Manjaro Linux 18.0.4, i7-7700 @ 3.60GHz, RTX 2070 (desktop)

## Build instruction
This project uses [CMake](https://cmake.org/) build system and [Conan](https://conan.io/) package manager. To successfully build the project, you need both tools installed. You can install conan through python `pip`:

``` shell
$ pip install conan # Or pip3 on some linux distributions, use sudo if nessesory
```

Adter installing all the tools, build the project with the following CMake instructions.
``` shell
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```

## Features
- Wind, gravity, and restoration forces simulation in compute shader with Euler's method
- frustum and distance cullings in compute shader with indirect drawing
- Tessellation LOD base on distance
- a pair of tessellation control shader and tessellation evaluation shader to generate triangle geometry
- An immediate GUI interface for user control

## Q & A
- Q: I don't see any grass.
  A: Make sure not to use the Intel integrated GPU to run the program

## Todo
- acceleration structure
- Orientation culling
- Terrain generation
- Infinite lawn with LOD
- Performance analysis
- Occlusion culling
- Better Grass Material
