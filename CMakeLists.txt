cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (AT2_all)

option(BUILD_TESTING "Use GTest" OFF)
option(USE_ASSIMP "Use ASSIMP library" OFF)
# option(DO_CLONE_GLTF_SAMPLE_MODELS "Automatically clone the https://github.com/KhronosGroup/glTF-Sample-Models.git to resources folder" OFF)
option(USE_SDL_INSTEADOF_GLFW "When enabled, SDL will be used instead of SFML" OFF)
option(USE_PLATFORM_HACKS "speed-up execution in exchange for some non-portable code" OFF)

set(ACTIVE_RENDERER "OpenGL" CACHE STRING "Renderer backend to use") # right now just one
set_property(CACHE ACTIVE_RENDERER PROPERTY STRINGS OpenGL Metal)

# Setup conan packet manager

# Download automatically, you can also just copy the conan.cmake file
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message (STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file (DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/0.17.0/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake" 
                 TLS_VERIFY ON)
endif()

include (${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run (   CONANFILE conanfile.py 
                    BUILD missing BASIC_SETUP
                    OPTIONS
                        "use_sdl = ${USE_SDL_INSTEADOF_GLFW}"
                        "use_gtest = ${BUILD_TESTING}"
                        "use_assimp = ${USE_ASSIMP}"
)

add_subdirectory ("src/AT2/")
add_subdirectory ("applications/examples/")
# add_subdirectory ("applications/sandbox/")
# add_subdirectory ("applications/test_task/")

if(BUILD_TESTING)
    message ("Testing enabled")
    add_subdirectory("src/Tests")
endif()
