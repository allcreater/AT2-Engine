#include "GlRenderer.h"
#include "../StateManager.h"

using namespace AT2;



unsigned int GlRendererCapabilities::GetMaxNumberOfTextureUnits() const
{
	GLint numTexUnits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTexUnits); //GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS ???

	if (numTexUnits < 0)
		throw AT2Exception(AT2Exception::ErrorCase::Renderer, "renderer capabilities query error");

	return numTexUnits;
}
unsigned int GlRendererCapabilities::GetMaxNumberOfColorAttachements() const
{
	GLint numRenderTargets = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &numRenderTargets);

	if (numRenderTargets < 0)
		throw AT2Exception(AT2Exception::ErrorCase::Renderer, "renderer capabilities query error");

	return numRenderTargets;
}
unsigned int GlRendererCapabilities::GetMaxTextureSize() const
{
	GLint maxTextureSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

	if (maxTextureSize < 0)
		throw AT2Exception(AT2Exception::ErrorCase::Renderer, "renderer capabilities query error");

	return maxTextureSize;
}
unsigned int GlRendererCapabilities::GetMaxNumberOfVertexAttributes() const
{
	GLint maxVertexAttribs = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);

	if (maxVertexAttribs < 0)
		throw AT2Exception(AT2Exception::ErrorCase::Renderer, "renderer capabilities query error");

	return maxVertexAttribs;
}

GlRenderer::GlRenderer()
{
	//TODO: return it
	//glDebugMessageCallback(GlErrorCallback, this);
	//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	m_rendererCapabilities = std::make_unique<GlRendererCapabilities>();
	m_resourceFactory = std::make_unique<GlResourceFactory>(this);
	m_stateManager = std::make_unique<StateManager>(*m_rendererCapabilities.get());
}

GlRenderer::~GlRenderer()
{
}


void GlRenderer::FinishFrame()
{
	glFinish();
	//SDL_GL_SwapWindow(m_window);
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
