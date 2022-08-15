set(METAL_SOURCES_LIST
    "AT2lowlevel.h"
    "FrameBuffer.h"
    "FrameBuffer.cpp"
    "Mappings.h"
    "Renderer.h"
    "Renderer.cpp"
    "ResourceFactory.h"
    "ResourceFactory.cpp"
    "ShaderProgram.h"
    "ShaderProgram.cpp"
    "MetalCpp.cpp"
    "MtlStateManager.h"
    "MtlStateManager.cpp"
    "PipelineState.h"
    "PipelineState.cpp"
    "ProgramIntrospection.h"
    "ProgramIntrospection.cpp"
    "Texture.h"
    "Texture.cpp"
    "VertexArray.h"
    "VertexArray.cpp"
    "Buffer.h"
    "Buffer.cpp"
)

#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX AT2 FILES ${METAL_SOURCES_LIST})


target_sources(${PROJECT_NAME} PRIVATE ${METAL_SOURCES_LIST})

message ("Apple frameworks will be linked")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/3rd_party/metal-cpp/")

find_library(QUARTZ_CORE_LIB QuartzCore)
find_library(METAL_LIB Metal)
find_library(FOUNDATION_LIB Foundation)

if ((NOT QUARTZ_CORE_LIB) OR (NOT METAL_LIB) OR (NOT FOUNDATION_LIB) )
message(FATAL_ERROR "Apple frameworks not found")
endif()


target_link_libraries(${PROJECT_NAME} PRIVATE ${QUARTZ_CORE_LIB} ${METAL_LIB} ${FOUNDATION_LIB})

