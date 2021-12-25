cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

option(USE_PLATFORM_HACKS "speed-up execution in exchange for some non-portable code" OFF)

add_subdirectory ("Core/")
add_subdirectory ("Platform/")
