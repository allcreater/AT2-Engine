cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# Hunter package manager inialization
include("gate/cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.20.24.tar.gz"
    SHA1 "3e2037a462bcf2ec3440f60298d933e74ffd4df3"
)
# end Hunter

project(AT2_all)

add_subdirectory ("src/AT2/")
add_subdirectory ("applications/sandbox/")
add_subdirectory ("applications/test_task/")

enable_testing()


#if(BUILD_TESTING)
    message ("Testing enabled")
    add_subdirectory("src/Tests")
#endif()