#include "GlRenderer.h"
#include "GlShaderProgram.h"
#include "GlTexture.h"
#include "GlVertexArray.h"

#include "../utils.hpp"

#include <fstream>
#include <optional>

#include "GlFrameBuffer.h"

using namespace std::literals;
using namespace AT2;

constexpr std::optional<GLint> DetermineInternalFormat(ExternalTextureFormat format)
{
    switch (format.ChannelsLayout)
    {
    case TextureLayout::Red:
        switch (format.DataType)
        {
        case BufferDataType::Byte:
        case BufferDataType::UByte: return GL_R8;
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_R16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_R32I; //TODO: check correctness
        case BufferDataType::HalfFloat: return GL_R16F;
        case BufferDataType::Float: return GL_R32F;
        default: return {};
        }

    case TextureLayout::RG:
        switch (format.DataType)
        {
        case BufferDataType::Byte:
        case BufferDataType::UByte: return GL_RG8;
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_RG16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_RG32I; //TODO: check correctness
        case BufferDataType::HalfFloat: return GL_RG16F;
        case BufferDataType::Float: return GL_RG32F;
        default: return {};
        }

    case TextureLayout::RGB:
    case TextureLayout::BGR:
        switch (format.DataType)
        {
        case BufferDataType::Byte:
        case BufferDataType::UByte: return GL_RGB8;
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_RGB16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_RGB32I; //TODO: check correctness
        case BufferDataType::HalfFloat: return GL_RGB16F;
        case BufferDataType::Float: return GL_RGB32F;
        default: return {};
        }

    case TextureLayout::RGBA:
    case TextureLayout::BGRA:
        switch (format.DataType)
        {
        case BufferDataType::Byte:
        case BufferDataType::UByte: return GL_RGBA8;
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_RGBA16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_RGBA32I; //TODO: check correctness
        case BufferDataType::HalfFloat: return GL_RGBA16F;
        case BufferDataType::Float: return GL_RGBA32F;
        default: return {};
        }

    case TextureLayout::DepthComponent:
        switch (format.DataType)
        {
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_DEPTH_COMPONENT16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_DEPTH_COMPONENT32; //TODO: check correctness
        case BufferDataType::Float: return GL_DEPTH_COMPONENT32F;
        default: return {};
        }

    default: return {};
    }
}


GlResourceFactory::GlResourceFactory(GlRenderer& renderer) : m_renderer(renderer) {}

IRenderer& GlResourceFactory::GetRenderer() noexcept
{
    return m_renderer;
}

std::shared_ptr<ITexture> GlResourceFactory::CreateTextureFromFramebuffer(const glm::ivec2& pos,
                                                                          const glm::uvec2& size) const
{
    auto texture = std::make_shared<GlTexture>(Texture2D {size},
                                               *DetermineInternalFormat(TextureFormats::RGBA8)); //TODO: choose formats?
    texture->CopyFromFramebuffer(0, pos, size);
    return texture;
}

std::shared_ptr<ITexture> GlResourceFactory::CreateTexture(const Texture& declaration,
                                                           ExternalTextureFormat desiredFormat) const
{
    const auto internalFormat = DetermineInternalFormat(desiredFormat);
    if (!internalFormat)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Unsupported texture format");

    return std::make_shared<GlTexture>(declaration, *internalFormat);
}

std::shared_ptr<IFrameBuffer> GlResourceFactory::CreateFrameBuffer() const
{
    return std::make_shared<GlFrameBuffer>(m_renderer.GetRendererCapabilities());
}

std::shared_ptr<IVertexArray> GlResourceFactory::CreateVertexArray() const
{
    return std::make_shared<GlVertexArray>(m_renderer.GetRendererCapabilities());
}

std::shared_ptr<IVertexBuffer> GlResourceFactory::CreateVertexBuffer(VertexBufferType type) const
{
    return std::make_shared<GlVertexBuffer>(type);
}

std::shared_ptr<IVertexBuffer> GlResourceFactory::CreateVertexBuffer(VertexBufferType type,
                                                                     size_t dataLength,
                                                                     const void* data) const
{
    auto buffer = CreateVertexBuffer(type);
    buffer->SetData(dataLength, data);
    return buffer;
}

//TODO: detach file as a shader source from specific implementation, remove inheritance
std::shared_ptr<IShaderProgram> GlResourceFactory::CreateShaderProgram() const
{
    return std::make_shared<GlShaderProgram>();
}