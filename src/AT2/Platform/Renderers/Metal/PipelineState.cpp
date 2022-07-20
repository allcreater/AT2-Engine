#include "PipelineState.h"

#include "Renderer.h"
#include "VertexArray.h"
#include "ShaderProgram.h"
#include "Mappings.h"

using namespace AT2;
using namespace AT2::Metal;

PipelineState::PipelineState(Renderer& renderer, const PipelineStateDescriptor& descriptor)
    : m_shaderProgram{std::dynamic_pointer_cast<ShaderProgram>(descriptor.GetShader())}
    , m_vertexArray{std::dynamic_pointer_cast<VertexArray>(descriptor.GetVertexArray())}
{
    assert(descriptor.GetShader());
    assert(descriptor.GetVertexArray());

    auto buildingState = ConstructMetalObject<MTL::RenderPipelineDescriptor>();

    {
        auto buildingDepthStencilState = ConstructMetalObject<MTL::DepthStencilDescriptor>();
        buildingDepthStencilState->setDepthWriteEnabled(descriptor.GetDepthState().WriteEnabled);
        buildingDepthStencilState->setDepthCompareFunction(descriptor.GetDepthState().TestEnabled ? Mappings::TranslateCompareFunction(descriptor.GetDepthState().CompareFunc) : MTL::CompareFunctionAlways);
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
    
    buildingState->setVertexDescriptor(m_vertexArray->GetVertexDescriptor().get());

    m_shaderProgram->Apply(*buildingState);

    {
        NS::Error* error;
        MTL::RenderPipelineReflection* reflection;
        m_pipelineState = Own(renderer.getDevice()->newRenderPipelineState(buildingState.get(), MTL::PipelineOptionArgumentInfo | MTL::PipelineOptionBufferTypeInfo, &reflection, &error));
        
        CheckErrors(error);
        
        m_shaderProgram->OnStateCreated(reflection);
    }

}
