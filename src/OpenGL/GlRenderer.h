#ifndef AT2_GL_RENDERER_H
#define AT2_GL_RENDERER_H

#include "AT2lowlevel.h"

namespace AT2
{

class GlRendererCapabilities : public IRendererCapabilities
{
public:
	unsigned int GetMaxNumberOfTextureUnits() const override;
	unsigned int GetMaxNumberOfColorAttachements() const override;
	unsigned int GetMaxTextureSize() const override;
	unsigned int GetMaxNumberOfVertexAttributes() const override;
};

class GlResourceFactory : public IResourceFactory
{
public:
	std::shared_ptr<ITexture> LoadTexture(const str& filename) const override;
	std::shared_ptr<ITexture> CreateTexture() const override;
};

class GlRenderer : public IRenderer
{
public:
	GlRenderer();
	virtual ~GlRenderer();

public:
	IResourceFactory* GetResourceFactory() const override			{ return m_resourceFactory; }
	IStateManager* GetStateManager() const override					{ return m_stateManager; }
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
	GlResourceFactory* m_resourceFactory;
	GlRendererCapabilities* m_rendererCapabilities;
};

}

#endif