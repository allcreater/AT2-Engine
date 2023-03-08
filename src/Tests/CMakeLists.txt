enable_testing()

include(GoogleTest)

set (TEST_SOURCES
"AABB.cpp"
"UI.cpp"
)

add_executable(AT2_Core_tests
    ${TEST_SOURCES}
)

target_include_directories (AT2_Core_tests PRIVATE "${CMAKE_SOURCE_DIR}/src/AT2/Core")

target_link_libraries(AT2_Core_tests PRIVATE AT2_Engine_Platform )

gtest_add_tests(TARGET      AT2_Core_tests
                TEST_SUFFIX .noArgs
                TEST_LIST   noArgsTests
)

target_compile_features(AT2_Core_tests PUBLIC cxx_std_20)
