cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (AT2_all)

# Download automatically, you can also just copy the conan.cmake file
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message (STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file (DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake" 
                 TLS_VERIFY ON)
endif()

include (${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run (   CONANFILE conanfile.py 
                    BUILD missing )

add_subdirectory ("src/AT2/")
add_subdirectory ("applications/sandbox/")
add_subdirectory ("applications/test_task/")

if(BUILD_TESTING)
    message ("Testing enabled")
    add_subdirectory("src/Tests")
endif()
