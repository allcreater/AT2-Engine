[![Build status](https://ci.appveyor.com/api/projects/status/rqwp0pju7ho1stkn?svg=true)](https://ci.appveyor.com/project/allcreater/at2-engine)

### What is
Simple graphics engine created for educational purpose.

### Features:
* cross-platform (in theory, sometimes could be actually broken :))
* abstraction layer for graphical API objects (right now only OpenGL 4.5 is supported)
* resource (textures, models, shaders) loading
* semiautomatical state managing, no need to manually bind textures
* c++17

### Two applications:
* sandbox is a playground for testing engine features
  * deferred renderer over simple scene graph
  * simple procedurally generated terrain with tesselaton
* test_task is a program that demonstates some UI and plot rendering (pretty obsolete)

### Dependencies:
could be obtained via *vcpkg*:
```vcpkg install glfw3 glew glm devil assimp gtest```
