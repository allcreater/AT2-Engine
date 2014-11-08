#include "GlTexture.h"

using namespace AT2;

GlTexture::GlTexture(TextureType type, GLint numComponents, GLenum format) :
	m_targetType(type),
	m_numComponents(numComponents),
	m_format(format),
	m_currentTextureModule(-1)
{
	glGenTextures(1, &m_id);

}

GlTexture::~GlTexture()
{
	glDeleteTextures(1, &m_id);
}

void GlTexture::Bind(unsigned int module)
{
	glActiveTexture(GL_TEXTURE0 + module);
	glBindTexture(static_cast<GLenum>(m_targetType), m_id);

	m_currentTextureModule = module;
}

void GlTexture::Unbind()
{
	m_currentTextureModule = -1;
}

void GlTexture::BuildMipmaps()
{
	glGenerateTextureMipmapEXT(m_id, static_cast<GLenum>(m_targetType));
}

//

GlTexture1D::GlTexture1D(GLint numComponents, GLenum format) : GlTexture(GlTexture::TextureType::Texture1D, numComponents, format)
{
}

void GlTexture1D::UpdateData(GLenum target, BufferData data)
{
	glTextureImage1DEXT(m_id, target, 0, m_format, data.Width, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	BuildMipmaps();

	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


GlTexture2D::GlTexture2D(GLint numComponents, GLenum format) : GlTexture(GlTexture::TextureType::Texture2D, numComponents, format)
{
}

void GlTexture2D::UpdateData(GLenum target, BufferData data)
{
	glTextureImage2DEXT(m_id, target, 0, m_format, data.Width, data.Height, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	BuildMipmaps();
	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


GlTexture3D::GlTexture3D(GLint numComponents, GLenum format) : GlTexture(GlTexture::TextureType::Texture3D, numComponents, format)
{
}

void GlTexture3D::UpdateData(GLenum target, BufferData data)
{
	glTextureImage3DEXT(m_id, target, 0, m_format, data.Width, data.Height, data.Depth, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	BuildMipmaps();
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

GlTextureCube::GlTextureCube(GLint numComponents, GLenum format) : GlTexture(GlTexture::TextureType::CubeMap, numComponents, format)
{
}

void GlTextureCube::UpdateData(GLenum target, BufferData data)
{
	glTextureImage2DEXT(m_id, target, 0, m_format, data.Width, data.Height, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	//BuildMipmaps();
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameterfEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}