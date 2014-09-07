#include "GlRenderer.h"
#include "../StateManager.h"
using namespace AT2;

GlRenderer::GlRenderer()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) /* Initialize SDL's Video subsystem */
        throw AT2Exception(AT2Exception::ErrorCase::Renderer, "SDL init failed");
 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
    /* Create our window centered at 512x512 resolution */
    m_window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 1024, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!m_window) /* Die if creation failed */
        throw AT2Exception(AT2Exception::ErrorCase::Renderer, "SDL: cannot create window");

	CheckSDLError();

	m_context = SDL_GL_CreateContext(m_window);

	CheckSDLError();

	SDL_GL_SetSwapInterval(1);

	glewExperimental = GL_TRUE;
	GLenum err=glewInit();
	if(err != GLEW_OK)
		throw AT2Exception(AT2Exception::ErrorCase::Renderer, "GLEW: cannot init");


	GLint numTexUnits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTexUnits); //GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS ???
	m_stateManager = new StateManager(numTexUnits);
	
}

GlRenderer::~GlRenderer()
{
}

void GlRenderer::CheckSDLError()
{
	const char *error = SDL_GetError();
	if (*error != '\0')
	{
		std::stringstream ss;
		ss << "SDL error: " << error << std::endl;
		SDL_ClearError();


		Log::Error() << ss.str();
		throw AT2Exception(AT2Exception::ErrorCase::Renderer, ss.str());
	}
}

ITexture* GlRenderer::CreateTexture(unsigned int _w, unsigned int _h, unsigned int _d, void* _data)
{
	if (_w > 0)
	{
		if (_h > 0)
		{
			if (_d > 0) // TEXTURE_3D
			{
			}
			else //TEXTURE_2D
			{
			}
		}
		else //TEXTURE_1D
		{
			if (_d > 0)
				std::logic_error("2d texture must have _w and _h");
		}
	}
	else
		throw std::logic_error("texture width must be positive");
}


void GlRenderer::SwapBuffers()
{
	SDL_GL_SwapWindow(m_window);
}

void GlRenderer::ClearBuffer(const glm::vec4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GlRenderer::Shutdown()
{
    SDL_GL_DeleteContext(m_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();

	m_context = nullptr;
	m_window = nullptr;
}