[![Build status](https://ci.appveyor.com/api/projects/status/rqwp0pju7ho1stkn?svg=true)](https://ci.appveyor.com/project/allcreater/at2-engine)

### What is
Simple graphics engine created for educational purpose.

### Features:
* cross-platform (in theory, sometimes could be actually broken :))
* abstraction layer for graphical API objects (right now only OpenGL 4.5 is supported)
* resource (textures, models, shaders) loading
* semiautomatical state managing, no need to manually bind textures
* c++20

### Two applications:
* sandbox is a playground for testing engine features
  * deferred renderer over simple scene graph
  * simple procedurally generated terrain with tesselaton
* test_task is a program that demonstates some UI and plot rendering (pretty obsolete)

### Dependencies:
Needs last version of C++20 compiler
Just ensure that you have [Conan package manager](https://conan.io/downloads.html) - all should be installed automatically.

Uses libraries:
* glfw3
* glm
* stb
* assimp
* gtest [optional]
* glad
* nlohmann-json
* [fx-gltf](https://github.com/jessey-git/fx-gltf)