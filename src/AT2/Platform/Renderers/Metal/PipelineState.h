#pragma once

#include "AT2lowlevel.h"

namespace AT2::Metal
{
    class Renderer;
    class ShaderProgram;
    class VertexArray;

    class PipelineState : public IPipelineState
    {
    public:
        PipelineState(Renderer& renderer, const PipelineStateDescriptor& descriptor);

// For internal usage
        void Apply(MTL::RenderCommandEncoder& commandEncoder);
        MTL::DepthStencilState* GetDepthStencilState() const noexcept { return m_depthStencilState.get(); }
        const std::shared_ptr<VertexArray>& GetVertexArray() const noexcept { return m_vertexArray; }
        const std::shared_ptr<ShaderProgram>& GetShaderProgram() const noexcept { return m_shaderProgram; }

    private:
        MtlPtr<MTL::RenderPipelineState> m_pipelineState;
        MtlPtr<MTL::DepthStencilState> m_depthStencilState;
        std::shared_ptr<VertexArray> m_vertexArray;
        std::shared_ptr<ShaderProgram> m_shaderProgram;

    };

}
