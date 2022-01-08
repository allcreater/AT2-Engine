#pragma once

#include <StateManager.h>

namespace AT2::Metal
{

class MtlStateManager : public StateManager
{
    using StateManager::StateManager;

    void ApplyState(RenderState state) override;
    
protected:
    void DoBind(const ITexture& texture, unsigned index) override;
    void DoBind(IFrameBuffer& texture) override;
    void DoBind(IShaderProgram& texture) override;
    void DoBind(IVertexArray& texture) override;
};

};
f
