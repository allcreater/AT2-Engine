project (AT2_UnitTests)

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)
find_package(GMock CONFIG REQUIRED)

include(GoogleTest)

include_directories ("${CMAKE_SOURCE_DIR}/src/")

file (GLOB ${PROJECT_NAME}_SOURCES
"*.cpp"
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC AT2_Engine GTest::main GMock::gmock
)


gtest_add_tests(TARGET      ${PROJECT_NAME}
                TEST_SUFFIX .noArgs
                TEST_LIST   noArgsTests
)