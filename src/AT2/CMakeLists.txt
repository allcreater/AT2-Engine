cmake_minimum_required (VERSION 3.7.1 FATAL_ERROR)

add_definitions(
#	-DUSE_ASSIMP
)


project (AT2_Engine)

hunter_add_package (glfw)
hunter_add_package (glm)
hunter_add_package (glew)
#hunter_add_package (Assimp)

find_package (glfw3 REQUIRED)
find_package (glm REQUIRED)
find_package (glew CONFIG REQUIRED)
find_package (DevIL REQUIRED)
#find_package (Assimp CONFIG REQUIRED)

#unfortunately DEVIL is not a Hunter package :(
if (DEVIL_FOUND)
    message("Trololo! ${IL_INCLUDE_DIR}")
    include_directories("${IL_INCLUDE_DIR}/../")
endif()

file(GLOB ${PROJECT_NAME}_SOURCES
"OpenGL/GLFW/*.cpp"
"OpenGL/*.cpp"
"UI/*.cpp"
"*.cpp"
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}  PRIVATE glfw glm ${IL_LIBRARIES} ${ILU_LIBRARIES} #DevIL ILU
                                       PUBLIC glew::glew) #Assimp::assimp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
