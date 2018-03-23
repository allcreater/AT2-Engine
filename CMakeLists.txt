cmake_minimum_required (VERSION 3.7.1 FATAL_ERROR)

# Hunter package manager inialization
include("gate/cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.20.24.tar.gz"
    SHA1 "3e2037a462bcf2ec3440f60298d933e74ffd4df3"
)
# end Hunter

add_definitions(
#	-DUSE_ASSIMP
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/libs/")
message("${CMAKE_SOURCE_DIR}/libs/")

project (AT2_Engine)

hunter_add_package (glfw)
hunter_add_package (glm)
hunter_add_package (glew)
#hunter_add_package (Assimp)

find_package (glfw3 REQUIRED)
find_package (glm REQUIRED)
find_package (glew CONFIG REQUIRED)
#find_package (DevIL REQUIRED)
#find_package (Assimp CONFIG REQUIRED)

#the temporary spike :(
include_directories("libs/DevIL SDK/include/")
link_directories ("libs/DevIL SDK/lib/x86/Release/")
#end temporary spike

file(GLOB ${PROJECT_NAME}_SOURCES
"src/OpenGL/GLFW/*.cpp"
"src/OpenGL/*.cpp"
"src/*.cpp"
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}  PRIVATE glfw glm DevIL ILU
                                       PUBLIC glew::glew) #Assimp::assimp)