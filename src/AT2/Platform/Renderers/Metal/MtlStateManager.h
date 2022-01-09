#pragma once

#include <StateManager.h>
#include "AT2lowlevel.h"

namespace AT2::Metal
{

class Renderer;

class MtlStateManager : public StateManager
{
    using StateManager::StateManager;

    void ApplyState(RenderState state) override;
    
protected:
    Renderer& GetRenderer() const;
    
    void DoBind(const ITexture& texture, unsigned index) override;
    void DoBind(IFrameBuffer& frameBuffer) override;
    void DoBind(IShaderProgram& shaderProgram) override;
    void DoBind(IVertexArray& vertexArray) override;
    
private:
};

};
