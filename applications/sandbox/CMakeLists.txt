project (AT2_sandbox)

file (GLOB ${PROJECT_NAME}_SOURCES
"*.cpp"
"../*.cpp" #for some utilities that are still not implemented at library
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/AT2/Core")
target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/AT2/Platform")
target_link_libraries(${PROJECT_NAME} PRIVATE AT2_Engine_Platform )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)