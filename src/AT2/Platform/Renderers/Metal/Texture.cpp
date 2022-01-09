#include "Texture.h"
#include "Renderer.h"
#include "Mappings.h"

using namespace AT2;
using namespace AT2::Metal;

namespace
{

constexpr size_t GetSizeofType(BufferDataType dataType)
{
    switch(dataType)
    {
        case BufferDataType::Byte:
        case BufferDataType::UByte:
            return 1;
            
        case BufferDataType::Short:
        case BufferDataType::UShort:
        case BufferDataType::HalfFloat:
            return 2;
            
        case BufferDataType::Int:
        case BufferDataType::UInt:
        case BufferDataType::Float:
        case BufferDataType::Fixed:
            return 4;
        
        case BufferDataType::Double:
            return 8;
            
        default:
            throw AT2Exception("Invalid BufferDataType");
    }
}

constexpr size_t GetNumberOfChannelsInLayout(TextureLayout layout)
{
    switch(layout)
    {
        case TextureLayout::Red:
            return 1;
        case TextureLayout::RG:
            return 2;
        case TextureLayout::RGB:
        case TextureLayout::BGR:
            return 3;
        case TextureLayout::RGBA:
        case TextureLayout::BGRA:
            return 4;
        case TextureLayout::DepthComponent:
        case TextureLayout::StencilIndex:
            return 1;
    }
}

constexpr size_t GetRowLength(ExternalTextureFormat format, size_t width)
{
    return width * GetSizeofType(format.DataType)*GetNumberOfChannelsInLayout(format.ChannelsLayout);
}

}

MtlTexture::MtlTexture(Renderer& renderer, Texture flavor, MTL::PixelFormat format) : m_flavor(flavor)
{
    auto* descriptor = MTL::TextureDescriptor::alloc()->init();
    descriptor->setPixelFormat(format);
    descriptor->setMipmapLevelCount(1);
    descriptor->setHeight(1);
    descriptor->setDepth(1);
    descriptor->setArrayLength(1);
    descriptor->setSampleCount(1);
    descriptor->setCpuCacheMode(MTL::CPUCacheModeDefaultCache);
    descriptor->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    //descriptor->setResourceOptions(MTL::ResourceStorageModePrivate);
    //descriptor->setStorageMode(MTL::StorageModePrivate);
    
    //descriptor->setTextureType(Mappings::TranslateTextureTarget(flavor));
    
    
    std::visit(
        Utils::overloaded {
            [&](const Texture1D& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1, 1};
                
                descriptor->setTextureType(MTL::TextureType1D);
                descriptor->setWidth(m_size.x);
                descriptor->setMipmapLevelCount(texture.getLevels());
                
                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture1DArray& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                
                descriptor->setTextureType(MTL::TextureType1DArray);
                descriptor->setWidth(m_size.x);
                descriptor->setArrayLength(m_size.y);
                descriptor->setMipmapLevelCount(texture.getLevels());
                
                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2D& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                
                descriptor->setTextureType(MTL::TextureType2D);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setMipmapLevelCount(texture.getLevels());
                
                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DMultisample& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                
                descriptor->setTextureType(MTL::TextureType2DMultisample);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setSampleCount(texture.getSamples());

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear));
            },
            [=](const Texture2DRectangle& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                
                descriptor->setTextureType(MTL::TextureType2D);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setMipmapLevelCount(texture.getLevels());
                
                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                
                descriptor->setTextureType(MTL::TextureType2DArray);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setArrayLength(m_size.z);
                descriptor->setMipmapLevelCount(texture.getLevels());
                
                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture2DMultisampleArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                
                descriptor->setTextureType(MTL::TextureType2DMultisampleArray);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setArrayLength(m_size.z);
                
                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear));
            },
            [&](const TextureCube& texture) {
                m_size = glm::ivec3 {texture.getSize(), 1};
                
                descriptor->setTextureType(MTL::TextureTypeCube);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setMipmapLevelCount(texture.getLevels());
                
                SetWrapMode( TextureWrapParams::Uniform(TextureWrapMode::ClampToEdge));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const TextureCubeArray& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                
                descriptor->setTextureType(MTL::TextureTypeCubeArray);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setArrayLength(m_size.z);
                descriptor->setMipmapLevelCount(texture.getLevels());

                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::ClampToEdge));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            },
            [&](const Texture3D& texture) {
                m_size = glm::ivec3 {texture.getSize()};
                
                descriptor->setTextureType(MTL::TextureType3D);
                descriptor->setWidth(m_size.x);
                descriptor->setHeight(m_size.y);
                descriptor->setDepth(m_size.z);
                descriptor->setMipmapLevelCount(texture.getLevels());
                
                SetWrapMode(TextureWrapParams::Uniform(TextureWrapMode::Repeat));
                SetSamplingMode(TextureSamplingParams::Uniform(TextureSamplingMode::Linear, texture.getLevels() > 1));
            }},
        flavor);
    
    m_texture = renderer.getDevice()->newTexture(descriptor);
    descriptor->release();
}

MtlTexture::~MtlTexture()
{
	
}

void MtlTexture::BindAsImage(unsigned int unit, glm::u32 level, glm::u32 layer, bool isLayered, BufferUsage usage) const
{
	
}

void MtlTexture::BuildMipmaps()
{
	
}

void MtlTexture::SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, const void* data)
{
    if (Mappings::TranslateExternalFormat(dataFormat) != m_texture->pixelFormat())
        throw AT2TextureException("SubImage's data format must be same as texture's internal format");
    
    assert(std::holds_alternative<Texture1D>(m_flavor));//TODO: properly support all alternatives
    
    m_texture->replaceRegion(MTL::Region(offset, size), level, data, 0);
}

void MtlTexture::SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, const void* data)
{
    if (Mappings::TranslateExternalFormat(dataFormat) != m_texture->pixelFormat())
        throw AT2TextureException("SubImage's data format must be same as texture's internal format");
    
    assert(std::holds_alternative<Texture2D>(m_flavor) || std::holds_alternative<Texture2DRectangle>(m_flavor));
    
    m_texture->replaceRegion(MTL::Region(offset.x, offset.y, size.x, size.y), level, data, GetRowLength(dataFormat, size.x));
}

void MtlTexture::SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, const void* data)
{
    if (Mappings::TranslateExternalFormat(dataFormat) != m_texture->pixelFormat())
        throw AT2TextureException("SubImage's data format must be same as texture's internal format");
    
    assert(std::holds_alternative<Texture3D>(m_flavor));
    
    m_texture->replaceRegion(MTL::Region(offset.x, offset.y, offset.z, size.x, size.y, size.z), level, data, GetRowLength(dataFormat, size.x));
}

void MtlTexture::CopyFromFramebuffer(int _level, glm::ivec2 pos, glm::ivec2 size, glm::ivec3 MtlTextureOffset)
{
	
}

void MtlTexture::SetWrapMode(TextureWrapParams wrapParams)
{
    
}

void MtlTexture::SetSamplingMode(TextureSamplingParams samplingParams)
{
	
}

void MtlTexture::SetAnisotropy(float anisotropy)
{
	
}

float MtlTexture::GetAnisotropy() const noexcept
{
    return 0.0f;
}
