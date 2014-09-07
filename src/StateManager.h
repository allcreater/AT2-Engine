#ifndef AT2_STATE_MANAGER
#define AT2_STATE_MANAGER

#include "AT2.h"

namespace AT2
{

class StateManager : public IStateManager
{
public:
	StateManager(unsigned int numTextureUnits);

public:
	virtual void BindTextures(const TextureSet& _textures);
	virtual void BindFramebuffer(const std::shared_ptr<IFrameBuffer>& _framebuffer);
	virtual void BindShader(const std::shared_ptr<IShaderProgram>& _shader);
	virtual void BindVertexArray(const std::shared_ptr<IVertexArray>& _vertexArray);

private:
	std::vector<std::shared_ptr<ITexture>> m_bindedTextures;
	std::shared_ptr<IShaderProgram> m_activeShader;
	std::shared_ptr<IFrameBuffer> m_activeFramebuffer;
	std::shared_ptr<IVertexArray> m_activeVertexArray;
};

}

#endif