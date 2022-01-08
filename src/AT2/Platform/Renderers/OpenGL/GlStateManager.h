#ifndef GL_STATE_MANAGER_H
#define GL_STATE_MANAGER_H

#include <StateManager.h>

namespace AT2::OpenGL
{

class GlStateManager : public StateManager
{
    using StateManager::StateManager;

    void ApplyState(RenderState state) override;

protected:
    void DoBind(const ITexture& texture, unsigned index) override;
    void DoBind(IFrameBuffer& framebuffer) override;
    void DoBind(IShaderProgram& shader) override;
    void DoBind(IVertexArray& vertexArray) override;
};

};

#endif