#include "GlTexture.h"

using namespace AT2;

GlTexture::GlTexture(TextureType type, GLint internalFormat) :
	m_targetType(type),
	m_internalFormat(internalFormat)
{
	glGenTextures(1, &m_id);

	DetermineExternalFormatAndDataType();
}

AT2::GlTexture::GlTexture(GLuint id, TextureType type, GLint internalFormat) :
	m_id(id),
	m_targetType(type),
	m_internalFormat(internalFormat)
{
	glGenTextures(1, &m_id);

	DetermineExternalFormatAndDataType();
}
GlTexture::GlTexture(TextureType type, GLint internalFormat, GLenum format, GLenum dataType) :
	m_targetType(type),
	m_internalFormat(internalFormat),
	m_format(format),
	m_dataType(dataType)
{
	glGenTextures(1, &m_id);
}

GlTexture::~GlTexture()
{
	glDeleteTextures(1, &m_id);
}

void GlTexture::DetermineExternalFormatAndDataType()
{
	GLint value;
	//auto-detect internal format
	
	glGetInternalformativ(static_cast<GLenum>(m_targetType), m_internalFormat, GL_TEXTURE_IMAGE_FORMAT, 1, &value);
	m_format = value;
	//auto-detect target type
	glGetInternalformativ(static_cast<GLenum>(m_targetType), m_internalFormat, GL_TEXTURE_IMAGE_TYPE, 1, &value);
	m_dataType = value;
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
GlTexture1D::GlTexture1D(GLint _internalFormat, GLuint _size, GLuint _levels) : GlTexture(GlTexture::TextureType::Texture1D, _internalFormat)
{
	const GLenum target = static_cast<GLenum>(m_targetType);
	m_size = glm::uvec3(_size, 1, 1);
	glTextureStorage1DEXT(m_id, target, _levels, _internalFormat, _size);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (_levels > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, _levels);
}

void GlTexture1D::SetData(GLuint _level, BufferData _data)
{
	const GLenum extFormat = _data.ExternalFormat ? _data.ExternalFormat : m_format, extDataType = _data.DataType ? _data.DataType : m_dataType;
	glTextureSubImage1DEXT(m_id, static_cast<GLenum>(m_targetType), _level, m_internalFormat, _data.Width, extFormat, extDataType, _data.Data);
}


GlTexture2D::GlTexture2D(GLint _internalFormat, glm::uvec2 _size, GLuint _levels) :
	GlTexture(GlTexture::TextureType::Texture2D, _internalFormat)
{
	const GLenum target = static_cast<GLenum>(m_targetType);

	m_size = glm::uvec3(_size, 1);
	glTextureStorage2DEXT(m_id, target, _levels, _internalFormat, _size.x, _size.y);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (_levels > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, _levels);
}

AT2::GlTexture2D::GlTexture2D(GLuint _id, GLint _internalFormat, glm::uvec2 _size, GLuint _levels) :
	GlTexture(_id, GlTexture::TextureType::Texture2D, _internalFormat)
{
	const GLenum target = static_cast<GLenum>(m_targetType);

	m_size = glm::uvec3(_size, 1);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GlTexture2D::SetData(GLuint _level, BufferData _data)
{
	const GLenum extFormat = _data.ExternalFormat ? _data.ExternalFormat : m_format, extDataType = _data.DataType ? _data.DataType : m_dataType;

	glTextureSubImage2DEXT(m_id, static_cast<GLenum>(m_targetType), _level, 0, 0, _data.Width, _data.Height, extFormat, extDataType, _data.Data);
}

void GlTexture2D::CopyFromFramebuffer(GLuint _level, glm::ivec2 pos, glm::uvec2 size)
{
	glCopyTextureSubImage2DEXT(m_id, static_cast<GLenum>(m_targetType), _level, 0, 0, pos.x, pos.y, size.x, size.y);
}

GlTexture2DArray::GlTexture2DArray(GLint _internalFormat, glm::uvec3 _size, GLuint _levels) :
	GlTexture(GlTexture::TextureType::Texture2DArray, _internalFormat)
{
	const GLenum target = static_cast<GLenum>(m_targetType);

	m_size = _size;
	glTextureStorage3DEXT(m_id, target, _levels, _internalFormat, _size.x, _size.y, _size.z);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, _levels);
}

void GlTexture2DArray::SetLayer(GLuint _level, GLuint _layer, BufferData _data)
{
	const GLenum extFormat = _data.ExternalFormat ? _data.ExternalFormat : m_format, extDataType = _data.DataType ? _data.DataType : m_dataType;
	glTextureSubImage3DEXT(m_id, static_cast<GLenum>(m_targetType), _level, 0, 0, _layer, _data.Width, _data.Height, 1, extFormat, extDataType, _data.Data);
}

GlTexture3D::GlTexture3D(GLint _internalFormat, glm::uvec3 _size, GLuint _levels) : GlTexture(GlTexture::TextureType::Texture3D, _internalFormat)
{
	const GLenum target = static_cast<GLenum>(m_targetType);
	m_size = _size;
	glTextureStorage3DEXT(m_id, target, _levels, _internalFormat, _size.x, _size.y, _size.z);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTextureParameteriEXT(m_id, static_cast<GLenum>(m_targetType), GL_TEXTURE_MIN_FILTER, (_levels > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTextureParameteriEXT(m_id, static_cast<GLenum>(m_targetType), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, static_cast<GLenum>(m_targetType), GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteriEXT(m_id, static_cast<GLenum>(m_targetType), GL_TEXTURE_MAX_LEVEL, _levels);
}

void GlTexture3D::SetLayer(GLuint _level, GLuint _layer, BufferData _data)
{
	const GLenum target = static_cast<GLenum>(m_targetType);
	const GLenum extFormat = _data.ExternalFormat ? _data.ExternalFormat : m_format, extDataType = _data.DataType ? _data.DataType : m_dataType;
	glTextureSubImage3DEXT(m_id, target, _level, 0, 0, _layer, _data.Width, _data.Height, 1, extFormat, extDataType, _data.Data);
}

void GlTexture3D::SetData(GLuint _level, BufferData _data)
{
	const GLenum target = static_cast<GLenum>(m_targetType);

	glTextureSubImage3DEXT(m_id, target, _level, 0, 0, 0, _data.Width, _data.Height, _data.Depth, m_format, m_dataType, _data.Data);
}

GlTextureCube::GlTextureCube(GLint _internalFormat, glm::uvec2 _size, GLuint _levels) : GlTexture(GlTexture::TextureType::CubeMap, _internalFormat)
{
	const GLenum target = static_cast<GLenum>(m_targetType);
	m_size = glm::uvec3(_size, 1);
	glTextureStorage2DEXT(m_id, target, _levels, _internalFormat, _size.x, _size.y);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (_levels > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_BASE_LEVEL, 0);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, _levels);
}


void GlTextureCube::SetFace(CubeMapFace _face, GLuint _level, BufferData _data)
{
	const GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<GLenum>(_face);
	const GLenum extFormat = _data.ExternalFormat ? _data.ExternalFormat : m_format, extDataType = _data.DataType ? _data.DataType : m_dataType;
	glTextureSubImage2DEXT(m_id, target, _level, 0, 0, _data.Width, _data.Height, extFormat, extDataType, _data.Data);
}
