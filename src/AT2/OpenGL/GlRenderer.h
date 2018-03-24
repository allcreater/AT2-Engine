#ifndef AT2_GL_RENDERER_H
#define AT2_GL_RENDERER_H

#include "AT2lowlevel.h"

namespace AT2
{
	
class GlRenderer;

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
	GlResourceFactory(GlRenderer* renderer);
	~GlResourceFactory();

public:
	std::shared_ptr<ITexture> LoadTexture(const str& filename) const override;
	std::shared_ptr<ITexture> CreateTexture() const override;
	std::shared_ptr<IVertexArray> CreateVertexArray() const override;
	std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type) const override;
	std::shared_ptr<IShaderProgram> CreateShaderProgramFromFiles(std::initializer_list<str> files) const override;
	void ReloadResources(ReloadableGroup group) override;

protected:
	GLint GetInternalFormat(GLuint externalFormat, GLuint externalType) const;
	std::shared_ptr<ITexture> LoadTexture_GLI(const str& filename) const;
	std::shared_ptr<ITexture> LoadTexture_DevIL(const str& filename) const;

private:
	GlRenderer* m_renderer;
	mutable std::vector<std::weak_ptr<IReloadable>> m_reloadableResourcesList;
};

class GlRenderer : public IRenderer
{
public:
	GlRenderer();
	virtual ~GlRenderer();

public:
	IResourceFactory& GetResourceFactory() const override				{ return *m_resourceFactory.get(); }
	IStateManager& GetStateManager() const override						{ return *m_stateManager.get(); }
	IRendererCapabilities& GetRendererCapabilities() const override		{ return *m_rendererCapabilities.get(); }

	void Shutdown() override;

	virtual void ClearBuffer(const glm::vec4& color);
	virtual void ClearDepth(float depth);
	virtual void FinishFrame();


private:
	static void  __stdcall GlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const GLvoid * userParam);

private:
	std::unique_ptr<IStateManager> m_stateManager;
	std::unique_ptr<IResourceFactory> m_resourceFactory;
	std::unique_ptr<GlRendererCapabilities> m_rendererCapabilities;
};

}

#endif