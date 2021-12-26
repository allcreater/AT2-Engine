#include "Texture.h"

using namespace AT2::Metal;

MtlTexture::MtlTexture(Texture flavor) : m_flavor(flavor)
{
	
}

MtlTexture::~MtlTexture()
{
	
}

void MtlTexture::Bind(unsigned int unit) const
{
	
}

void MtlTexture::BindAsImage(unsigned int unit, glm::u32 level, glm::u32 layer, bool isLayered, BufferUsage usage) const
{
	
}

void MtlTexture::Unbind() const
{
	
}

void MtlTexture::BuildMipmaps()
{
	
}

void MtlTexture::SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, const void* data)
{
	
}

void MtlTexture::SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, const void* data)
{
}

void MtlTexture::SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, const void* data)
{
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
