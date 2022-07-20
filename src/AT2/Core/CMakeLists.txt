cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (AT2_Engine_Core)

set (${PROJECT_NAME}_SOURCES
    "DataLayout/BufferLayout.h"
    "DataLayout/BufferLayout.cpp"
    "DataLayout/StructuredBuffer.h"
    "DataLayout/StructuredBuffer.cpp"
    "DataLayout/IO.hpp"

    "Resources/GltfSceneLoader.h"
    "Resources/GltfSceneLoader.cpp"
    "Resources/MeshLoader.h"
    "Resources/MeshLoader.cpp"
    "Resources/TextureLoader.h"
    "Resources/TextureLoader_devIL.cpp"
    "Resources/TextureLoader_stb.cpp"

    "Scene/Animation.h"
    "Scene/Animation.cpp"
    "Scene/Channel.h"
    "Scene/Scene.h"
    "Scene/Scene.cpp"

    "UI/InputHelper.h"
    "UI/InputHelper.cpp"
    "UI/Plot.cpp"
    "UI/StackPanel.cpp"
    "UI/UI.h"
    "UI/UI.cpp"

    "AABB.h"
    "BufferMapperGuard.h"
    "Camera.h"
    "log.cpp"
    "log.h"
    "lru_cache.h"
    "matrix_stack.h"
    "Mesh.h"
    "UniformContainer.h"
    "UniformContainer.cpp"
    "utils.hpp"

    "../AT2.h"
    "../AT2_exceptions.hpp"
    "../AT2_states.hpp"
    "../AT2_textures.hpp"
    "../AT2_types.hpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. PREFIX AT2 FILES ${${PROJECT_NAME}_SOURCES})

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
    $<$<BOOL:${USE_ASSIMP}>:USE_ASSIMP>
    $<$<BOOL:${USE_PLATFORM_HACKS}>:USE_PLATFORM_HACKS>
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/fx-gltf/include/")


if(DEVIL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DEVIL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${IL_LIBRARIES} ${ILU_LIBRARIES}) 
endif()


target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
