[![Build status](https://ci.appveyor.com/api/projects/status/rqwp0pju7ho1stkn?svg=true)](https://ci.appveyor.com/project/allcreater/at2-engine)

## What is
Simple graphics engine created for educational purpose.

## Features:
* aim on code readability and extensibility 
* cross-platform (tested on Windows, Ubuntu and MacOS)
* abstraction layer for different graphical APIs:
  * OpenGL 4.5 renderer
  * Metal renderer powered by [Apple Metal-cpp library](https://developer.apple.com/metal/cpp/) (WIP, by state of 19.01.2022 could render only one example)
* two IO abstraction backends:
  * GLFW
  * SDL
* resource (textures, models, shaders) loading
* semiautomatical state managing
* c++20

## Examples:
* examples - contains just one example with rotating cube, but thinking as set of examples/tests
* sandbox is a playground for testing engine features
  * deferred renderer over simple scene graph
  * simple procedurally generated terrain with tesselaton
* test_task is a program that demonstates some UI and plot rendering (pretty obsolete)

## Dependencies:
Needs last version of C++20 compiler (may build under Visual Studio 16.10, XCode 13 and Clang 13)  
Also check if you have [Conan package manager](https://conan.io/downloads.html) - almost* all should be installed automatically.

> The only exception is MacOS-specific *Metal-cpp* which has no official repository and requires small patch from [here](https://github.com/gzorin/sdl-metal-cpp-example) [will be automated soon]

### Uses libraries:
* GLFW / SDL
* glm
* stb
* assimp [optional]
* gtest [optional]
* glad
* nlohmann-json
* [fx-gltf](https://github.com/jessey-git/fx-gltf)
* [Metal-cpp](https://developer.apple.com/metal/cpp/) [optional, Mac only]