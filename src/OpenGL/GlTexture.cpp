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
	glBindTexture((GLenum)m_targetType, m_id);


	m_currentTextureModule = module;
}

void GlTexture::Unbind()
{
	m_currentTextureModule = -1;
}


//

GlTexture1D::GlTexture1D(GLint numComponents, GLenum format) : GlTexture(GlTexture::TextureType::Texture1D, numComponents, format)
{
}

void GlTexture1D::UpdateData(GLenum target, BufferData data)
{
	//glTextureImage1DEXT(m_id, target, 0, m_numComponents, data.Width, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	glBindTexture(static_cast<GLenum>(m_targetType), m_id);
	glTexImage1D(target, 0, m_numComponents, data.Width, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	glGenerateMipmap(target);

	glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	
}


GlTexture2D::GlTexture2D(GLint numComponents, GLenum format) : GlTexture(GlTexture::TextureType::Texture2D, numComponents, format)
{
}

void GlTexture2D::UpdateData(GLenum target, BufferData data)
{
	glBindTexture(static_cast<GLenum>(m_targetType), m_id);
	//glTextureImage2DEXT(m_id, target, 0, m_numComponents, data.Width, data.Height, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	glTexImage2D(target, 0, m_numComponents, data.Width, data.Height, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	glGenerateMipmap(static_cast<GLenum>(m_targetType));
	
	glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


GlTexture3D::GlTexture3D(GLint numComponents, GLenum format) : GlTexture(GlTexture::TextureType::Texture3D, numComponents, format)
{
}

void GlTexture3D::UpdateData(GLenum target, BufferData data)
{
	glBindTexture(static_cast<GLenum>(m_targetType), m_id);
	//glTextureImage3DEXT(m_id, target, 0, m_numComponents, data.Width, data.Height, data.Depth, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	glTexImage3D(target, 0, m_format, data.Width, data.Height, data.Depth, 0, m_format, GL_UNSIGNED_BYTE, data.Data);
	glGenerateMipmap(static_cast<GLenum>(m_targetType));

	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}