#include "PipelineState.h"

#include "Renderer.h"
#include "VertexArray.h"
#include "ShaderProgram.h"
#include "Mappings.h"

using namespace AT2;
using namespace AT2::Metal;

namespace
{
    MtlPtr<MTL::VertexDescriptor> MakeVertexDescriptor(const VertexArrayDescriptor& descriptor)
    {
        auto vertexDescriptor = ConstructMetalObject<MTL::VertexDescriptor>();

        for (size_t index = 0; const auto& layout : descriptor.GetBufferLayouts())
        {
            auto mtlBufferLayout = vertexDescriptor->layouts()->object(index++);

            if (!layout.Enabled)
                continue;

            mtlBufferLayout->setStride(layout.Stride);
            mtlBufferLayout->setStepRate(layout.StepRate);
            mtlBufferLayout->setStepFunction(Mappings::TranslateVertexStepFunc(layout.StepFunc));
        }

        for (size_t index = 0; const auto& layout : descriptor.GetVertexAttributeLayouts())
        {
            auto* mtlAttribute = vertexDescriptor->attributes()->object(index++);

            if (!layout.Enabled)
                continue;

            mtlAttribute->setFormat(Mappings::TranslateVertexFormat(layout.Type, layout.Count, layout.IsNormalized));
            mtlAttribute->setOffset(layout.Offset);
            mtlAttribute->setBufferIndex(layout.BufferIndex);
        }


        return vertexDescriptor;
    }
}


PipelineState::PipelineState(Renderer& renderer, const PipelineStateDescriptor& descriptor)
    : m_shaderProgram{std::dynamic_pointer_cast<ShaderProgram>(descriptor.GetShader())}
{
    assert(descriptor.GetShader());

    auto buildingState = ConstructMetalObject<MTL::RenderPipelineDescriptor>();

    {
        auto buildingDepthStencilState = ConstructMetalObject<MTL::DepthStencilDescriptor>();
        buildingDepthStencilState->setDepthWriteEnabled(descriptor.GetDepthState().WriteEnabled);
        buildingDepthStencilState->setDepthCompareFunction(descriptor.GetDepthState().TestEnabled ? Mappings::TranslateCompareFunction(descriptor.GetDepthState().CompareFunc) : MTL::CompareFunctionAlways);

        m_depthStencilState = Own(renderer.getDevice()->newDepthStencilState(buildingDepthStencilState.get()));
    }

    {
        //TODO take actual attachment layout from active render stage?
        auto* attachment = buildingState->colorAttachments()->object(0);
        attachment->setBlendingEnabled(descriptor.GetBlendMode().Enabled);
        attachment->setSourceRGBBlendFactor(Mappings::TranslateBlendFactor(descriptor.GetBlendMode().SourceFactor));
        attachment->setSourceAlphaBlendFactor(Mappings::TranslateBlendFactor(descriptor.GetBlendMode().SourceFactor));
        attachment->setDestinationRGBBlendFactor(Mappings::TranslateBlendFactor(descriptor.GetBlendMode().DestinationFactor));
        attachment->setDestinationAlphaBlendFactor(Mappings::TranslateBlendFactor(descriptor.GetBlendMode().DestinationFactor));
        attachment->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    }

    buildingState->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);
    
    buildingState->setVertexDescriptor(MakeVertexDescriptor(descriptor.GetVertexArrayDescriptor()).get());

    m_shaderProgram->Apply(*buildingState);

    {
        NS::Error* error;
        MTL::RenderPipelineReflection* reflection;
        m_pipelineState = Own(renderer.getDevice()->newRenderPipelineState(buildingState.get(), MTL::PipelineOptionArgumentInfo | MTL::PipelineOptionBufferTypeInfo, &reflection, &error));
        
        CheckErrors(error);
        
        m_shaderProgram->OnStateCreated(reflection);
    }

}

void PipelineState::Apply(MTL::RenderCommandEncoder& renderEncoder)
{
    renderEncoder.setRenderPipelineState(m_pipelineState.get());
    renderEncoder.setDepthStencilState(m_depthStencilState.get());
}