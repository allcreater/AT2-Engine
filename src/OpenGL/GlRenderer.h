#ifndef AT2_GL_RENDERER_H
#define AT2_GL_RENDERER_H

#include "AT2lowlevel.h"

namespace AT2
{

class GlRendererCapabilities : public IRendererCapabilities
{
public:
	unsigned int GetMaxNumberOfTextureUnits() const;
	unsigned int GetMaxNumberOfColorAttachements() const;
	unsigned int GetMaxTextureSize() const;
	unsigned int GetMaxNumberOfVertexAttributes() const;
};

class GlRenderer : public IRenderer
{
public:
	GlRenderer();
	virtual ~GlRenderer();

public:
	ITexture* CreateTexture(unsigned int w, unsigned int h, unsigned int d, void* data) override;

	IStateManager* GetStateManager() const override { return m_stateManager; }
	IRendererCapabilities* GetRendererCapabilities() const override { return m_rendererCapabilities; }

	void Shutdown() override;

	virtual void ClearBuffer(const glm::vec4& color);
	virtual void ClearDepth(float depth);
	virtual void SwapBuffers();

private:
	void CheckSDLError();

private:
    SDL_Window* m_window;
    SDL_GLContext m_context;

	IStateManager* m_stateManager;
	GlRendererCapabilities* m_rendererCapabilities;
};

}

#endif