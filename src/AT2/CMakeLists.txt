cmake_minimum_required (VERSION 3.7.1 FATAL_ERROR)

add_definitions(
    -DUSE_ASSIMP
    -DGLFW_INCLUDE_NONE
    -DGLEW_NO_GLU
)

project (AT2_Engine)

find_package (glfw3 CONFIG REQUIRED)
find_package (glm CONFIG REQUIRED)
find_package (GLEW REQUIRED)
find_package (DevIL REQUIRED)
find_package (Assimp REQUIRED)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
"*.cpp"
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw assimp::assimp ${IL_LIBRARIES} ${ILU_LIBRARIES}
                         PUBLIC GLEW::GLEW glm) 

set_target_properties(${PROJECT_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_EXTENSIONS OFF
)