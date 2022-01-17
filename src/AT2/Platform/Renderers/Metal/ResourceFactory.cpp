#include "ResourceFactory.h"

#include "Renderer.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "ShaderProgram.h"
#include "Mappings.h"

using namespace AT2;
using namespace AT2::Metal;

ResourceFactory::ResourceFactory(Renderer& renderer) : m_renderer(renderer)
{
	
}

std::shared_ptr<ITexture> ResourceFactory::CreateTextureFromFramebuffer(const glm::ivec2& pos, const glm::uvec2& size) const
{
    return std::make_shared<MtlTexture>(m_renderer, Texture2D{size}, MTL::PixelFormatRGBA8Unorm);
}

std::shared_ptr<ITexture> ResourceFactory::CreateTexture(const Texture& declaration, ExternalTextureFormat desiredFormat) const
{
    return std::make_shared<MtlTexture>(m_renderer, declaration, Mappings::TranslateExternalFormat(desiredFormat));
}

std::shared_ptr<IFrameBuffer> ResourceFactory::CreateFrameBuffer() const
{
    return std::make_shared<FrameBuffer>(m_renderer, m_renderer.GetRendererCapabilities().GetMaxNumberOfColorAttachments());
}

std::shared_ptr<IVertexArray> ResourceFactory::CreateVertexArray() const
{
    return std::make_shared<VertexArray>(m_renderer.GetRendererCapabilities());
}

std::shared_ptr<IBuffer> ResourceFactory::CreateBuffer(VertexBufferType type) const
{
    return std::make_shared<Buffer>(m_renderer, type);
}

std::shared_ptr<IBuffer> ResourceFactory::CreateBuffer(VertexBufferType type, std::span<const std::byte> data) const
{
    auto buffer = CreateBuffer(type);
    buffer->SetDataRaw(data);
    return buffer;
}

std::shared_ptr<IShaderProgram> ResourceFactory::CreateShaderProgramFromFiles(std::initializer_list<str> files) const
{
    constexpr char source[] = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct VertexIn
        {
            float3 position [[ attribute(1) ]];
            float2 texCoord [[ attribute(2) ]];
        };

        struct VertexUniforms
        {
            float4x4 u_matModelView [[id(0)]];
            float4x4 u_matProjection;
        };

        vertex float4 vertex_main(
            const VertexIn vertex_in        [[ stage_in ]],
            constant VertexUniforms& params [[ buffer(0) ]]
        )
        {
            const auto viewSpacePos = params.u_matModelView * float4(vertex_in.position, 1);
            return params.u_matProjection * viewSpacePos;
        }

        fragment float4 fragment_main(texture2d<float, access::sample> texAlbedo [[texture(0)]])
        {
            return float4(1, 0, 0, 1);
        }
    )";
    
    static std::shared_ptr<ShaderLibrary> library = std::make_shared<ShaderLibrary>(m_renderer, source);
    
    ShaderProgram::Descriptor descriptor {library, "vertex_main", "fragment_main"};
    
    return std::make_shared<ShaderProgram>(descriptor);
}

void AT2::Metal::ResourceFactory::ReloadResources(ReloadableGroup group)
{
	
}
