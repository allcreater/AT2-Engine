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
    "SDL/application.cpp"
    "SDL/graphics_context.h"
    "SDL/graphics_context.cpp"
    "SDL/window.h"
    "SDL/window.cpp"
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
    "Renderers/OpenGL/GlBuffer.h"
    "Renderers/OpenGL/GlBuffer.cpp"
    "Renderers/OpenGL/Mappings.h"
)

set(METAL_SOURCES_LIST
    "Renderers/Metal/AT2lowlevel.h"
    "Renderers/Metal/FrameBuffer.h"
    "Renderers/Metal/FrameBuffer.cpp"
    "Renderers/Metal/Mappings.h"
    "Renderers/Metal/Renderer.h"
    "Renderers/Metal/Renderer.cpp"
    "Renderers/Metal/ResourceFactory.h"
    "Renderers/Metal/ResourceFactory.cpp"
    "Renderers/Metal/ShaderProgram.h"
    "Renderers/Metal/ShaderProgram.cpp"
    "Renderers/Metal/MetalCpp.cpp"
    "Renderers/Metal/MtlStateManager.h"
    "Renderers/Metal/MtlStateManager.cpp"
    "Renderers/Metal/Texture.h"
    "Renderers/Metal/Texture.cpp"
    "Renderers/Metal/UniformBuffer.h"
    "Renderers/Metal/UniformBuffer.cpp"
    "Renderers/Metal/VertexArray.h"
    "Renderers/Metal/VertexArray.cpp"
    "Renderers/Metal/Buffer.h"
    "Renderers/Metal/Buffer.cpp"
)

set(COMMON_SOURCES_LIST
    "Application.h"
    "callback_types.h"
    "callback_types.cpp"
    "Common.h"
    "Foundation.h"
    "Foundation.cpp"
)

if (USE_SDL_INSTEADOF_GLFW)
    list(APPEND AT2_PLATFORM_SOURCES_LIST ${SDL_SOURCES_LIST})
else()
    list(APPEND AT2_PLATFORM_SOURCES_LIST ${GLFW_SOURCES_LIST})
endif()
list(APPEND AT2_PLATFORM_SOURCES_LIST 
    ${COMMON_SOURCES_LIST}
    ${OPENGL_SOURCES_LIST}
)

# Gathering all together into AT2_PLATFORM_SOURCES_LIST
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND AT2_PLATFORM_SOURCES_LIST
        ${METAL_SOURCES_LIST}
        "Application_Metal.cpp"
    )
else()
    list(APPEND AT2_PLATFORM_SOURCES_LIST
        "Application.cpp"
    )
endif()


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX AT2 FILES ${AT2_PLATFORM_SOURCES_LIST})

add_library(${PROJECT_NAME} STATIC ${AT2_PLATFORM_SOURCES_LIST})

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

# MacOS - specific
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message ("Apple frameworks will be linked")
  target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/3rd_party/metal-cpp/")
  
  find_library(QUARTZ_CORE_LIB QuartzCore)
  find_library(METAL_LIB Metal)
  find_library(FOUNDATION_LIB Foundation)
  
  if ((NOT QUARTZ_CORE_LIB) OR (NOT METAL_LIB) OR (NOT FOUNDATION_LIB) )
    message(FATAL_ERROR "Apple frameworks not found")
  endif()

  
  target_link_libraries(${PROJECT_NAME} PRIVATE ${QUARTZ_CORE_LIB} ${METAL_LIB} ${FOUNDATION_LIB})
  
endif()
