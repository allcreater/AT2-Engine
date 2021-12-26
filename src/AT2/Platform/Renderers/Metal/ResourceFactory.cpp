#include "ResourceFactory.h"

#include "Texture.h"
#include "FrameBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "ShaderProgram.h"

using namespace AT2::Metal;

ResourceFactory::ResourceFactory(Renderer& renderer) : m_renderer(renderer)
{
	
}

std::shared_ptr<ITexture> ResourceFactory::CreateTextureFromFramebuffer(const glm::ivec2& pos, const glm::uvec2& size) const
{
    return std::shared_ptr<ITexture>();
}

std::shared_ptr<ITexture> ResourceFactory::CreateTexture(const Texture& declaration, ExternalTextureFormat desiredFormat) const
{
    return std::make_shared<MtlTexture>(declaration, 0);
}

std::shared_ptr<IFrameBuffer> ResourceFactory::CreateFrameBuffer() const
{
    return std::make_shared<Framebuffer>(m_renderer.GetRendererCapabilities());
}

std::shared_ptr<IVertexArray> ResourceFactory::CreateVertexArray() const
{
    return std::make_shared<VertexArray>(m_renderer.GetRendererCapabilities());
}

std::shared_ptr<IVertexBuffer> ResourceFactory::CreateVertexBuffer(VertexBufferType type) const
{
    return std::make_shared<VertexBuffer>();
}

std::shared_ptr<IVertexBuffer> ResourceFactory::CreateVertexBuffer(VertexBufferType type, std::span<const std::byte> data) const
{
    auto buffer = CreateVertexBuffer(type);
    buffer->SetDataRaw(data);
    return buffer;
}

std::shared_ptr<IShaderProgram> ResourceFactory::CreateShaderProgramFromFiles(std::initializer_list<str> files) const
{
    return std::make_shared<ShaderProgram>();
}

void AT2::Metal::ResourceFactory::ReloadResources(ReloadableGroup group)
{
	
}
