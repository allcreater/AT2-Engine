#pragma once

#include "AT2lowlevel.h"

namespace AT2::OpenGL
{
    class GlRenderer;
    
    //Do nothing, all work actual happens at GlStateManager
    class PipelineState : public IPipelineState
    {
    public:
        PipelineState(GlRenderer& renderer, const PipelineStateDescriptor& descriptor) : m_pipelineDescriptor{descriptor} {}

// For internal usage
        const PipelineStateDescriptor& GetDescriptor() const noexcept { return m_pipelineDescriptor; }

    private:
        PipelineStateDescriptor m_pipelineDescriptor;

    };

}
