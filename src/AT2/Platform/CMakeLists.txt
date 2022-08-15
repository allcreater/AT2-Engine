cmake_minimum_required (VERSION 3.14 FATAL_ERROR)

project (AT2_Engine_Platform)

set(COMMON_SOURCES_LIST
    "Application.h"
    "callback_types.h"
    "callback_types.cpp"
    "Common.h"
    "Foundation.h"
    "Foundation.cpp"
    "GraphicsContextInterface.h"
)

add_library(${PROJECT_NAME} STATIC ${COMMON_SOURCES_LIST})

if (USE_SDL_INSTEADOF_GLFW)
    add_subdirectory("SDL")
else()
    add_subdirectory("GLFW")
endif()

#add_subdirectory("Renderers/${ACTIVE_RENDERER}")
add_subdirectory("Renderers/OpenGL")

if (${ACTIVE_RENDERER} STREQUAL "Metal")
    add_subdirectory("Renderers/Metal")

    target_sources(${PROJECT_NAME} PRIVATE
    "Application_Metal.cpp"
    )
else()
    target_sources(${PROJECT_NAME} PRIVATE
    "Application.cpp"
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX AT2 FILES ${SOURCES})

target_compile_definitions( ${PROJECT_NAME} PRIVATE
    $<$<BOOL:${USE_PLATFORM_HACKS}>:USE_PLATFORM_HACKS>
    $<$<BOOL:${USE_SDL_INSTEADOF_GLFW}>:USE_SDL_INSTEADOF_GLFW>
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")


target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS} PRIVATE AT2_Engine_Core)

if(DEVIL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DEVIL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${IL_LIBRARIES} ${ILU_LIBRARIES}) 
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

