cmake_minimum_required (VERSION 3.7.1 FATAL_ERROR)

add_definitions(
    -DUSE_ASSIMP
    -DGLFW_INCLUDE_NONE
    -DGLEW_NO_GLU
)

project (AT2_Engine) 
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package (glfw REQUIRED)
find_package (glm REQUIRED)
find_package (glad REQUIRED)
find_package (stb REQUIRED)
find_package (assimp REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
    "*.h"
    "*.hpp"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw::glfw assimp::assimp stb::stb
                         PUBLIC glad::glad glm::glm) 
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)