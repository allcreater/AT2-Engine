cmake_minimum_required (VERSION 3.7.1 FATAL_ERROR)

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
