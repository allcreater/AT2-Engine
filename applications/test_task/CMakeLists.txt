add_definitions(
#	-DUSE_ASSIMP
)

project (Test_Task)

#hunter_add_package (Assimp)
#find_package (Assimp CONFIG REQUIRED)


include_directories ("${CMAKE_SOURCE_DIR}/src/")

file (GLOB ${PROJECT_NAME}_SOURCES
"*.cpp"
"../*.cpp" #for some utilities that are still not implemented at library
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME} AT2_Engine) #Assimp::assimp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)