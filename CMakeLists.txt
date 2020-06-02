cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(AT2_all)

add_subdirectory ("src/AT2/")
add_subdirectory ("applications/sandbox/")
add_subdirectory ("applications/test_task/")

if(BUILD_TESTING)
    message ("Testing enabled")
    add_subdirectory("src/Tests")
endif()
