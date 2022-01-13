#pragma once

#include <StateManager.h>
#include "AT2lowlevel.h"

namespace AT2::Metal
{

class Renderer;

class MtlStateManager : public StateManager
{
public:
    using StateManager::StateManager;

    void ApplyState(RenderState state) override;
    
    // for internal usage
    void OnStartRendering(MTL::RenderCommandEncoder* renderEncoder) {m_renderEncoder = renderEncoder; }
    void OnFinishRendering(){ m_renderEncoder = nullptr; }
    
    MtlPtr<MTL::RenderPipelineState> GetOrBuildState();
    
protected:
    Renderer& GetRenderer() const;
    
    void DoBind(ITexture& texture, unsigned index) override;
    void DoBind(IShaderProgram& shaderProgram) override;
    void DoBind(IVertexArray& vertexArray) override;
    
private:
    MTL::RenderCommandEncoder* m_renderEncoder;
    MtlPtr<MTL::RenderPipelineDescriptor> m_buildingState = ConstructMetalObject<MTL::RenderPipelineDescriptor>();
    MtlPtr<MTL::DepthStencilDescriptor> m_buildingDepthStencilState;
    MtlPtr<MTL::RenderPipelineState> m_currentState;
    
    bool m_stateInvalidated = true;
    bool m_buildingDepthStencilStateInvalidated = true;
};

};
