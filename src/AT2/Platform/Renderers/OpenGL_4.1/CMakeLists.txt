set(OPENGL_SOURCES_LIST
    "AT2lowlevel.h"
    "GlFrameBuffer.h"
    "GlFrameBuffer.cpp"
    "GlPipelineState.h"
    "GlProgramIntrospection.h"
    "GlProgramIntrospection.cpp"
    "GlRenderer.h"
    "GlRenderer.cpp"
    "GlResourceFactory.cpp"
    "GlShaderProgram.h"
    "GlShaderProgram.cpp"
    "GlStateManager.h"
    "GlStateManager.cpp"
    "GlTexture.h"
    "GlTexture.cpp"
    "GlVertexArray.h"
    "GlVertexArray.cpp"
    "GlBuffer.h"
    "GlBuffer.cpp"
    "Mappings.h"
)

target_sources(${PROJECT_NAME} PRIVATE ${OPENGL_SOURCES_LIST})