#ifndef AT2_GL_RENDERER_H
#define AT2_GL_RENDERER_H

#include "AT2lowlevel.h"

namespace AT2
{
	
class GlRenderer;
class GlTexture;
class GlFrameBuffer;


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
	std::shared_ptr<ITexture> CreateTextureFromFramebuffer(const glm::ivec2& pos, const glm::uvec2& size) const override;
	std::shared_ptr<ITexture> CreateTexture(const Texture& declaration, ExternalTextureFormat desiredFormat) const override;
	std::shared_ptr<IVertexArray> CreateVertexArray() const override;
	std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType) const override;
	std::shared_ptr<IVertexBuffer> CreateVertexBuffer(VertexBufferType type, const BufferTypeInfo& dataType, size_t dataLength, const void* data) const override;
	std::shared_ptr<IShaderProgram> CreateShaderProgramFromFiles(std::initializer_list<str> files) const override;
	void ReloadResources(ReloadableGroup group) override;

protected:
	static GLint GetInternalFormat(GLuint externalFormat, GLuint externalType);

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
	IResourceFactory& GetResourceFactory() const override				{ return *m_resourceFactory; }
	IStateManager& GetStateManager() const override						{ return *m_stateManager; }
	IRendererCapabilities& GetRendererCapabilities() const override		{ return *m_rendererCapabilities; }

	void Shutdown() override;

	void Draw(Primitives::Primitive type, long int first, long int count, int numInstances = 1, int baseVertex = 0) override;
	void SetViewport(const AABB2d& viewport) override;
	void ClearBuffer(const glm::vec4& color) override;
	void ClearDepth(float depth) override;
	void FinishFrame() override;

	IFrameBuffer& GetDefaultFramebuffer() const override;

private:
	static void GlErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const GLvoid * userParam);

private:
	std::unique_ptr<IStateManager> m_stateManager;
	std::unique_ptr<IResourceFactory> m_resourceFactory;
	std::unique_ptr<GlRendererCapabilities> m_rendererCapabilities;
};

}

#endif
