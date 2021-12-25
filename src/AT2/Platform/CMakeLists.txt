cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

project (AT2_Engine_Platform)

set(${PROJECT_NAME}_SOURCES
    "GLFW/glfw_application.h"
    "GLFW/glfw_application.cpp"
    "GLFW/glfw_window.h"
    "GLFW/glfw_window.cpp"

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

    "Application.h"
    "Application.cpp"
    "callback_types.h"
    "callback_types.cpp"
    "Common.h"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX AT2 FILES ${${PROJECT_NAME}_SOURCES})

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_SOURCES}
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
    $<$<BOOL:${USE_PLATFORM_HACKS}>:USE_PLATFORM_HACKS>
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../")


target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS} PRIVATE AT2_Engine_Core)

if(DEVIL_FOUND)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DEVIL)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${IL_LIBRARIES} ${ILU_LIBRARIES}) 
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)