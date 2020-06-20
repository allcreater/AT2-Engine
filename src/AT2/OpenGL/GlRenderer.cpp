#include "GlRenderer.h"

#include "Mappings.h"
#include "GlFrameBuffer.h"
#include "../StateManager.h"

using namespace AT2;

static int GetInteger(GLenum parameter, GLint min = std::numeric_limits<GLint>::min(), GLint max = std::numeric_limits<GLint>::max())
{
	GLint result = 0;
	glGetIntegerv(parameter, &result); 

	if (result < min || result > max)
		throw AT2Exception(AT2Exception::ErrorCase::Renderer, "renderer capabilities query error");

	return result;

}

unsigned int GlRendererCapabilities::GetMaxNumberOfTextureUnits() const
{
	return GetInteger(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 1); //GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, GL_MAX_TEXTURE_IMAGE_UNITS ???
}
unsigned int GlRendererCapabilities::GetMaxNumberOfColorAttachements() const
{
	return GetInteger(GL_MAX_COLOR_ATTACHMENTS, 1);
}
unsigned int GlRendererCapabilities::GetMaxTextureSize() const
{
	return GetInteger(GL_MAX_TEXTURE_SIZE, 1);
}
unsigned int GlRendererCapabilities::GetMaxNumberOfVertexAttributes() const
{
	return GetInteger(GL_MAX_VERTEX_ATTRIBS, 1);
}

GlRenderer::GlRenderer()
{
    glDebugMessageCallback(GlErrorCallback, this);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    m_rendererCapabilities = std::make_unique<GlRendererCapabilities>();
    m_resourceFactory = std::make_unique<GlResourceFactory>(this);
    m_stateManager = std::make_unique<StateManager>(*m_rendererCapabilities);
}

GlRenderer::~GlRenderer()
{
}


void GlRenderer::FinishFrame()
{
	glFinish();
	//SDL_GL_SwapWindow(m_window);
}

IFrameBuffer& GlRenderer::GetDefaultFramebuffer() const
{
	return GlScreenFrameBuffer::Get();
}

void GlRenderer::ClearBuffer(const glm::vec4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GlRenderer::ClearDepth(float depth)
{
	glClearDepth(depth);
	glClear(GL_DEPTH_BUFFER_BIT);
}


void GlRenderer::Shutdown()
{
}

void GlRenderer::DispatchCompute(glm::uvec3 threadGroupSize)
{
    glDispatchCompute(threadGroupSize.x, threadGroupSize.y, threadGroupSize.z);
}

//sub-optimal but abstract :)
void GlRenderer::Draw(Primitives::Primitive type, long first, long count, int numInstances, int baseVertex)
{
    if (first < 0 || count < 0 || numInstances < 0 || baseVertex < 0)
        throw AT2Exception(AT2Exception::ErrorCase::Renderer, "GlRenderer: Draw arguments should be positive!");

    const auto platformPrimitiveMode = Mappings::TranslatePrimitiveType(type);

    if (const auto patchParams = std::get_if<Primitives::Patches>(&type))
        glPatchParameteri(GL_PATCH_VERTICES, patchParams->NumControlPoints);

    if ( const auto indexDataType = GetStateManager().GetIndexDataType())
    {
        const auto platformIndexBufferType = Mappings::TranslateExternalType(*indexDataType);
        if (numInstances > 1)
        {
            glDrawElementsInstancedBaseVertex(platformPrimitiveMode, static_cast<GLsizei>(count),
                                              platformIndexBufferType, reinterpret_cast<void *>(first),
                                              static_cast<GLsizei>(numInstances),
                                              baseVertex);
        }
        else
        {
            glDrawElementsBaseVertex(platformPrimitiveMode, static_cast<GLsizei>(count), platformIndexBufferType,
                                     reinterpret_cast<void *>(first), baseVertex);
        }
    }
    else
    {
        if (baseVertex != 0)
            throw AT2Exception(AT2Exception::ErrorCase::Renderer, "GlRenderer: baseVertex supported only with index buffers");

        if (numInstances > 1)
            glDrawArraysInstanced(platformPrimitiveMode, static_cast<GLint>(first), static_cast<GLsizei>(count), numInstances);
        else
            glDrawArrays(platformPrimitiveMode, static_cast<GLint>(first), static_cast<GLsizei>(count));
    }
}

void GlRenderer::SetViewport(const AABB2d& viewport)
{
	glViewport(
		static_cast<GLint>(viewport.MinBound.x),
		static_cast<GLint>(viewport.MinBound.y),
		static_cast<GLint>(viewport.GetWidth()),
		static_cast<GLint>(viewport.GetHeight())
	);
}

void GlRenderer::GlErrorCallback(GLenum _source, GLenum _type, GLuint _id, GLenum _severity, GLsizei _length, const GLchar * _message, const GLvoid * _userParam)
{
	auto rendererInstance = reinterpret_cast<const GlRenderer*>(_userParam);

	assert(dynamic_cast<const GlRenderer*>(rendererInstance));

	switch (_severity)
	{
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		Log::Info() << "OpenGL[N]: " << _message << std::endl; break;
	case GL_DEBUG_SEVERITY_LOW:
		Log::Debug() << "OpenGL[L]: " << _message << std::endl; break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		Log::Warning() << "OpenGL[M]: " << _message << std::endl; break;
	case GL_DEBUG_SEVERITY_HIGH:
		Log::Error() << "OpenGL[H]: " << _message << std::endl; break;
	default:
		assert(false); //WTF?!
	}
}
