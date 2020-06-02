project (AT2_UnitTests)

enable_testing()

find_package(GTest MODULE REQUIRED)

include(GoogleTest)

include_directories ("${CMAKE_SOURCE_DIR}/src/")

file (GLOB ${PROJECT_NAME}_SOURCES
"*.cpp"
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE AT2_Engine GTest::gtest GTest::gtest_main 
)


gtest_add_tests(TARGET      ${PROJECT_NAME}
                TEST_SUFFIX .noArgs
                TEST_LIST   noArgsTests
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
