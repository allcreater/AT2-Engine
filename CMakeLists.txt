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

project (AT2_sandbox)


#hunter_add_package (Assimp)
#find_package (Assimp CONFIG REQUIRED)


#include_directories ("src/AT2/")

file (GLOB ${PROJECT_NAME}_SOURCES
"src/*.cpp"
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

add_subdirectory ("src/AT2/")

target_link_libraries(${PROJECT_NAME} AT2_Engine) #Assimp::assimp)