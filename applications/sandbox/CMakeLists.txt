add_definitions(
#	-DUSE_ASSIMP
)

project (AT2_sandbox)


#hunter_add_package (Assimp)
#find_package (Assimp CONFIG REQUIRED)


file (GLOB ${PROJECT_NAME}_SOURCES
"*.cpp"
"../*.cpp" #for some utilities that are still not implemented at library
)

add_executable(${PROJECT_NAME}
    ${${PROJECT_NAME}_SOURCES}
)

target_include_directories (${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/")

target_link_libraries(${PROJECT_NAME} AT2_Engine) #Assimp::assimp)
