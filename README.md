# OpenGL Grass Renderer
C++/OpenGL implementation of [Responsive Real-Time Grass Rendering for General 3D Scenes](https://www.cg.tuwien.ac.at/research/publications/2017/JAHRMANN-2017-RRTG/JAHRMANN-2017-RRTG-draft.pdf). Use a combination of Compute, Tessellation, and normal Vertex and Fragment shaders.

![demo image](image.gif)

## Build
``` shell
$ pip install conan # Or pip3 on some linux distributions
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```
