cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (AT2_Engine_Platform)

set(GLFW_SOURCES_LIST
    "GLFW/glfw_application.h"
    "GLFW/glfw_application.cpp"
    "GLFW/glfw_window.h"
    "GLFW/glfw_window.cpp"
)

set (SDL_SOURCES_LIST
    "SDL/application.h"
    "SDL/window.h"
)

set(OPENGL_SOURCES_LIST
    "Renderers/OpenGL/AT2lowlevel.h"
    "Renderers/OpenGL/GlFrameBuffer.h"
    "Renderers/OpenGL/GlFrameBuffer.cpp"
    "Renderers/OpenGL/GlProgramIntrospection.h"
    "Renderers/OpenGL/GlProgramIntrospection.cpp"
    "Renderers/OpenGL/GlRenderer.h"
    "Renderers/OpenGL/GlRenderer.cpp"
    "Renderers/OpenGL/GlResourceFactory.cpp"
    "Renderers/OpenGL/GlShaderProgram.h"
    "Renderers/OpenGL/GlShaderProgram.cpp"
    "Renderers/OpenGL/GlStateManager.h"
    "Renderers/OpenGL/GlStateManager.cpp"
    "Renderers/OpenGL/GlTexture.h"
    "Renderers/OpenGL/GlTexture.cpp"
    "Renderers/OpenGL/GlTimerQuery.h"
    "Renderers/OpenGL/GlTimerQuery.cpp"
    "Renderers/OpenGL/GlUniformBuffer.h"
    "Renderers/OpenGL/GlUniformBuffer.cpp"
    "Renderers/OpenGL/GlVertexArray.h"
    "Renderers/OpenGL/GlVertexArray.cpp"
    "Renderers/OpenGL/GlVertexBuffer.h"
    "Renderers/OpenGL/GlVertexBuffer.cpp"
    "Renderers/OpenGL/Mappings.h"
)

set(COMMON_SOURCES_LIST
    "Application.h"
    "callback_types.h"
    "callback_types.cpp"
    "Common.h"
    "Foundation.h"
)

if (USE_SDL_INSTEADOF_SFML)
    list(APPEND AT2_PLATFORM_SOURCES_LIST ${SDL_SOURCES_LIST})
else()
    list(APPEND AT2_PLATFORM_SOURCES_LIST ${GLFW_SOURCES_LIST})
endif()

list(APPEND AT2_PLATFORM_SOURCES_LIST
    ${COMMON_SOURCES_LIST}
    ${OPENGL_SOURCES_LIST}
    "Application.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX AT2 FILES ${AT2_PLATFORM_SOURCES_LIST})

add_library(${PROJECT_NAME} STATIC
    ${AT2_PLATFORM_SOURCES_LIST}
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
    $<$<BOOL:${USE_PLATFORM_HACKS}>:USE_PLATFORM_HACKS>
    $<$<BOOL:${USE_SDL_INSTEADOF_SFML}>:USE_SDL_INSTEADOF_SFML>
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")


target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS} PRIVATE AT2_Engine_Core)

if(DEVIL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DEVIL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${IL_LIBRARIES} ${ILU_LIBRARIES}) 
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)