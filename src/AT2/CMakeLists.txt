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
find_package (nlohmann_json REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
    "*.h"
    "*.hpp"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp stb::stb
                         PUBLIC glfw::glfw glad::glad glm::glm nlohmann_json::nlohmann_json) 


if(USE_DEVIL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DEVIL)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/fx-gltf/include/")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)