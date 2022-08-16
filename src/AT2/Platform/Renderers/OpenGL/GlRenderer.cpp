#include "GlRenderer.h"

#include "GlStateManager.h"
#include "GlFrameBuffer.h"
#include "Mappings.h"

using namespace std::literals;
using namespace AT2;
using namespace OpenGL;

namespace
{
    class GlRendererCapabilities : public IRendererCapabilities
    {
    public:
        [[nodiscard]] unsigned int GetMaxNumberOfTextureUnits() const override { return m_maxNumberOfTextureUnits; }
        [[nodiscard]] unsigned int GetMaxNumberOfColorAttachments() const override { return m_maxNumberOfColorAttachments; }
        [[nodiscard]] unsigned int GetMaxTextureSize() const override { return m_maxTextureSize; }
        [[nodiscard]] unsigned int GetMaxNumberOfVertexAttributes() const override { return m_maxNumberOfVertexAttributes; }

    private:
        unsigned int m_maxNumberOfTextureUnits =        GetInteger(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 1);
        unsigned int m_maxNumberOfColorAttachments =    GetInteger(GL_MAX_COLOR_ATTACHMENTS, 1);
        unsigned int m_maxTextureSize =                 GetInteger(GL_MAX_TEXTURE_SIZE, 1);
        unsigned int m_maxNumberOfVertexAttributes =    []
        {
            const auto maxAttribs = GetInteger(GL_MAX_VERTEX_ATTRIBS, 1);
            const auto maxBindings = GetInteger(GL_MAX_VERTEX_ATTRIB_BINDINGS, 1);
            if (maxAttribs != maxBindings)
            {
                Log::Warning() << "OpenGL renderer:"sv
                               << "GL_MAX_VERTEX_ATTRIBS and GL_MAX_VERTEX_ATTRIB_BINDINGS"sv
                               << "are not same. AT2 supports lesser value" << std::endl;
            }

            return std::min(maxAttribs, maxBindings);
        }();
    };

    void PrintDiagnosticInfo()
    {
        Log::Info() << "AT2 OpenGL renderer initialized. \n"sv
                    << "Renderer: "sv << glGetString(GL_RENDERER) << " ("sv << glGetString(GL_VERSION) << ")\n"sv
                    << "Vendor: "sv << glGetString(GL_VENDOR) << '\n'
                    << "GLSL version: "sv << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    }
} // namespace

GlRenderer::GlRenderer(IPlatformGraphicsContext& graphicsContext, GLADloadproc glFunctionsBinder) : m_graphicsContext {graphicsContext}
{
    if (!gladLoadGLLoader(glFunctionsBinder))
        throw AT2RendererException("Failed to initialize GLAD");

    glDebugMessageCallback(GlErrorCallback, this);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_FRAMEBUFFER_SRGB);

    PrintDiagnosticInfo();

    m_rendererCapabilities = std::make_unique<GlRendererCapabilities>();
    m_resourceFactory = std::make_unique<GlResourceFactory>(*this);
    m_stateManager = std::make_unique<GlStateManager>(*this);
    m_defaultFramebuffer = std::make_unique<GlScreenFrameBuffer>(*this);
}

void GlRenderer::BeginFrame()
{
}

void GlRenderer::FinishFrame()
{
    glFinish();
}

IFrameBuffer& GlRenderer::GetDefaultFramebuffer() const
{
    return *m_defaultFramebuffer;
}

void GlRenderer::DispatchCompute(const std::shared_ptr<IShaderProgram>& computeProgram, glm::uvec3 threadGroupSize)
{
    // TODO: special compute pipeline or something else
    auto temporaryPipeline = GetResourceFactory().CreatePipelineState(PipelineStateDescriptor().SetShader(computeProgram));
    GetStateManager().ApplyPipelineState(temporaryPipeline);

    glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); //TODO: more smart barriers
}

//sub-optimal but abstract :)
void GlRenderer::Draw(Primitives::Primitive type, size_t first, long count, int numInstances, int baseVertex)
{
    if (first < 0 || count < 0 || numInstances < 0 || baseVertex < 0)
        throw AT2RendererException( "GlRenderer: Draw arguments should be positive!");

    const auto platformPrimitiveMode = Mappings::TranslatePrimitiveType(type);

    if (auto* const patchParams = std::get_if<Primitives::Patches>(&type))
        glPatchParameteri(GL_PATCH_VERTICES, patchParams->NumControlPoints);

    if (const auto indexDataType = GetStateManager().GetIndexDataType())
    {
        const auto platformIndexBufferType = Mappings::TranslateExternalType(*indexDataType);
        if (numInstances > 1)
        {
            glDrawElementsInstancedBaseVertex(platformPrimitiveMode, static_cast<GLsizei>(count),
                                              platformIndexBufferType, reinterpret_cast<void*>(first),
                                              static_cast<GLsizei>(numInstances), baseVertex);
        }
        else
        {
            glDrawElementsBaseVertex(platformPrimitiveMode, static_cast<GLsizei>(count), platformIndexBufferType,
                                     reinterpret_cast<void*>(first), baseVertex);
        }
    }
    else
    {
        if (baseVertex != 0)
            throw AT2RendererException("GlRenderer: baseVertex supported only with index buffers");

        if (numInstances > 1)
            glDrawArraysInstanced(platformPrimitiveMode, static_cast<GLint>(first), static_cast<GLsizei>(count),
                                  numInstances);
        else
            glDrawArrays(platformPrimitiveMode, static_cast<GLint>(first), static_cast<GLsizei>(count));
    }
}

void GlRenderer::SetViewport(const AABB2d& viewport)
{
    glViewport(static_cast<GLint>(viewport.MinBound.x), static_cast<GLint>(viewport.MinBound.y),
               static_cast<GLint>(viewport.GetWidth()), static_cast<GLint>(viewport.GetHeight()));
}

void GlRenderer::SetScissorWindow(const AABB2d& window)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(static_cast<GLint>(window.MinBound.x), static_cast<GLint>(window.MinBound.y),
               static_cast<GLint>(window.GetWidth()), static_cast<GLint>(window.GetHeight()));
}

void GlRenderer::GlErrorCallback(GLenum _source, GLenum _type, GLuint _id, GLenum _severity, GLsizei _length,
                                 const GLchar* _message, const GLvoid* _userParam)
{
    const auto* const rendererInstance = static_cast<const GlRenderer*>(_userParam);
    assert(dynamic_cast<const GlRenderer*>(rendererInstance));

    switch (_severity)
    {
    case GL_DEBUG_SEVERITY_NOTIFICATION: Log::Info() << "OpenGL[N]: " << _message << std::endl; break;
    case GL_DEBUG_SEVERITY_LOW: Log::Debug() << "OpenGL[L]: " << _message << std::endl; break;
    case GL_DEBUG_SEVERITY_MEDIUM: Log::Warning() << "OpenGL[M]: " << _message << std::endl; break;
    case GL_DEBUG_SEVERITY_HIGH: Log::Error() << "OpenGL[H]: " << _message << std::endl; break;
    default: assert(false); //WTF?!
    }
}
