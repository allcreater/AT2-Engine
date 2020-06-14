#ifndef AT2_STATE_MANAGER
#define AT2_STATE_MANAGER

#include "AT2.h"

namespace AT2
{

class StateManager : public IStateManager
{
public:
	StateManager(const IRendererCapabilities& rendererCapabilities);

public:
	void BindTextures(const TextureSet& _textures) override;
	void BindFramebuffer(std::shared_ptr<IFrameBuffer> _framebuffer) override;
	void BindShader(std::shared_ptr<IShaderProgram> _shader) override;
	void BindVertexArray(std::shared_ptr<IVertexArray> _vertexArray) override;
	//TODO: BindIndexArray?

	//TextureSet& GetActiveTextures() const override;
	std::weak_ptr<IFrameBuffer> GetActiveFrameBuffer() const override;
	std::weak_ptr<IShaderProgram> GetActiveShader() const override;
	std::weak_ptr<IVertexArray> GetActiveVertexArray() const override;

	std::optional<BufferDataType> GetIndexDataType() const noexcept override { return m_activeIndexBufferType; }

private:
	std::vector<std::shared_ptr<ITexture>> m_bindedTextures;
	std::shared_ptr<IShaderProgram> m_activeShader;
	std::shared_ptr<IFrameBuffer> m_activeFramebuffer;
	std::shared_ptr<IVertexArray> m_activeVertexArray;

	std::optional<BufferDataType> m_activeIndexBufferType;
};

}

#endif