cmake_minimum_required (VERSION 3.7.1 FATAL_ERROR)

add_definitions(
    -DUSE_ASSIMP
    -DGLFW_INCLUDE_NONE
)

project (AT2_Engine)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
    "*.h"
    "*.hpp"
    "*.cpp"
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/")


target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

if(DEVIL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DEVIL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${IL_LIBRARIES} ${ILU_LIBRARIES}) 
endif()


target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/fx-gltf/include/")

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)