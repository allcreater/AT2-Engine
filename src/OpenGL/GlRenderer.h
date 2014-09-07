#ifndef AT2_GL_RENDERER_H
#define AT2_GL_RENDERER_H

#include "AT2lowlevel.h"

namespace AT2
{

class GlRenderer : public IRenderer
{
public:
	GlRenderer();
	virtual ~GlRenderer();

public:
	virtual ITexture* CreateTexture(unsigned int _w, unsigned int _h, unsigned int _d, void* _data);

	virtual IStateManager* GetStateManager() { return m_stateManager; }

	virtual void Shutdown();

	virtual void ClearBuffer(const glm::vec4& color);
	virtual void SwapBuffers();

private:
	void CheckSDLError();

private:
    SDL_Window* m_window;
    SDL_GLContext m_context;

	IStateManager* m_stateManager;
};

}

#endif