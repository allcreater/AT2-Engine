#include "GlTexture.h"
#include <optional>

using namespace AT2;

GLenum GlTexture::GetTarget() const
{
	return std::visit(Utils::overloaded{
		[=](const Texture1D& texture)
		{
			return GL_TEXTURE_1D;
		},
		[=](const Texture1DArray& texture)
		{
			return GL_TEXTURE_1D_ARRAY;
		},
		[=](const Texture2D& texture)
		{
			return GL_TEXTURE_2D;
		},
		[=](const Texture2DMultisample& texture)
		{
			return GL_TEXTURE_2D_MULTISAMPLE;
		},
		[=](const Texture2DRectangle& texture)
		{
			return GL_TEXTURE_RECTANGLE;
		},
		[=](const Texture2DArray& texture)
		{
			return GL_TEXTURE_2D_ARRAY;
		},
		[=](const Texture2DMultisampleArray& texture)
		{
			return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
		},
		[=](const TextureCube& texture)
		{
			return GL_TEXTURE_CUBE_MAP;
		},
		[=](const TextureCubeArray& texture)
		{
			return GL_TEXTURE_CUBE_MAP_ARRAY;
		},
		[=](const Texture3D& texture)
		{
			return GL_TEXTURE_3D;
		}
		}, m_flavor);
}

GlTexture::GlTexture(Texture flavor, GLint internalFormat) : m_flavor(flavor), m_internalFormat(internalFormat)
{
	glGenTextures(1, &m_id);

	const auto target = GetTarget();

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_BASE_LEVEL, 0);

	//TODO: test all cases
	//todo: remove EXT?
	std::visit(Utils::overloaded{
	    [=](const Texture1D& texture)
		{
	        glTextureStorage1DEXT(m_id, GL_TEXTURE_1D, texture.getLevels(), m_internalFormat , texture.getSize().x);

			glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
	    },
	    [=](const Texture1DArray& texture)
	    {
			glTextureStorage2DEXT(m_id, GL_TEXTURE_1D_ARRAY, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);
		    return ;
	    },
	    [=](const Texture2D& texture)
	    {
	        glTextureStorage2DEXT(m_id, GL_TEXTURE_2D, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);

	        glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	        glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
	    },
	    [=](const Texture2DMultisample& texture)
	    {
			const bool fixedSampleLocations = true; //TODO: move to texture declaration? Also levels are not using
			glTextureStorage2DMultisample(m_id, texture.getSamples(), m_internalFormat, texture.getSize().x, texture.getSize().y, fixedSampleLocations);

			//glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			//glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
			//
	    },
	    [=](const Texture2DRectangle& texture)
	    {
			glTextureStorage2DEXT(m_id, GL_TEXTURE_RECTANGLE, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);
	    },
	    [=](const Texture2DArray& texture)
	    {
            glTextureStorage3DEXT(m_id, GL_TEXTURE_2D_ARRAY, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z);

            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
	    },
	    [=](const Texture2DMultisampleArray& texture)
	    {
			const bool fixedSampleLocations = true; //TODO: move to texture declaration? Also levels are not using
			//std::unique_ptr<>
			glTextureStorage3DMultisample(m_id, texture.getSamples(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z, fixedSampleLocations);
	    },
	    [=](const TextureCube& texture)
	    {
			glTextureStorage2DEXT(m_id, GL_TEXTURE_CUBE_MAP, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y);

            glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
	    },
	    [=](const TextureCubeArray& texture)
	    {
			glTextureStorage3DEXT(m_id, GL_TEXTURE_CUBE_MAP_ARRAY, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z);
	    },
	    [=](const Texture3D& texture)
	    {
			glTextureStorage3DEXT(m_id, GL_TEXTURE_3D, texture.getLevels(), m_internalFormat, texture.getSize().x, texture.getSize().y, texture.getSize().z);
			glTextureParameteriEXT(m_id, target, GL_TEXTURE_MIN_FILTER, (texture.getLevels() > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTextureParameteriEXT(m_id, target, GL_TEXTURE_MAX_LEVEL, texture.getLevels());
	    }
    }, flavor);

}


GlTexture::~GlTexture()
{
	glDeleteTextures(1, &m_id);
}

//void GlTexture::DetermineExternalFormatAndDataType()
//{
//	GLint value;
//	//auto-detect internal format
//	
//	glGetInternalformativ(static_cast<GLenum>(m_targetType), m_internalFormat, GL_TEXTURE_IMAGE_FORMAT, 1, &value);
//	m_format = value;
//	//auto-detect target type
//	glGetInternalformativ(static_cast<GLenum>(m_targetType), m_internalFormat, GL_TEXTURE_IMAGE_TYPE, 1, &value);
//	m_dataType = value;
//}

void GlTexture::Bind(unsigned int module)
{
	glActiveTexture(GL_TEXTURE0 + module);
	glBindTexture(GetTarget(), m_id);

	m_currentTextureModule = module;
}

void GlTexture::Unbind()
{
	m_currentTextureModule = -1;
}

void GlTexture::BuildMipmaps()
{
	glGenerateTextureMipmapEXT(m_id, GetTarget());
}

glm::uvec3 GlTexture::GetSize() const
{

	//TODO: remake :/
	return std::visit(Utils::overloaded{
		[=](const Texture1D& texture)
		{
			return glm::uvec3 {texture.getSize().x, 1, 1};
		},
		[=](const Texture1DArray& texture)
		{
			return glm::uvec3 {texture.getSize(), 1};
		},
		[=](const Texture2D& texture)
		{
			return glm::uvec3 {texture.getSize(), 1};
		},
		[=](const Texture2DMultisample& texture)
		{
			return glm::uvec3 {texture.getSize(), 1};
		},
		[=](const Texture2DRectangle& texture)
		{
			return glm::uvec3 {texture.getSize(), 1};
		},
		[=](const Texture2DArray& texture)
		{
			return texture.getSize();
		},
		[=](const Texture2DMultisampleArray& texture)
		{
			return texture.getSize();
		},
		[=](const TextureCube& texture)
		{
			return glm::uvec3 {texture.getSize(), 1};
		},
		[=](const TextureCubeArray& texture)
		{
			return texture.getSize();
		},
		[=](const Texture3D& texture)
		{
			return texture.getSize();
		}
		}, m_flavor);
}

void GlTexture::SetWrapMode(TextureWrapMode wrapMode)
{
	//TODO: use dirty flag and set it on bind
    m_wrapMode = wrapMode;

	const auto target = GetTarget();
    const int mode = [=]
    {
		switch(wrapMode)
		{
		case TextureWrapMode::ClampToEdge:			return GL_CLAMP_TO_EDGE;
		case TextureWrapMode::ClampToBorder:		return GL_CLAMP_TO_BORDER;
		case TextureWrapMode::MirroredRepeat:		return GL_MIRRORED_REPEAT;
		case TextureWrapMode::Repeat:				return GL_REPEAT;
		case TextureWrapMode::MirrorClampToEdge:	return GL_MIRROR_CLAMP_TO_EDGE;
		default:
			assert(0);
		}
	}();

	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_S, mode);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_T, mode);
	glTextureParameteriEXT(m_id, target, GL_TEXTURE_WRAP_R, mode);
}

static GLenum translateExternalFormat(TextureLayout layout)
{
	switch (layout)
	{
	case TextureLayout::Red: return GL_RED;
	case TextureLayout::RG: return GL_RG;
	case TextureLayout::RGB: return GL_RGB;
	case TextureLayout::BGR: return GL_BGR;
	case TextureLayout::RGBA: return GL_RGBA;
	case TextureLayout::BGRA: return GL_BGRA;
	case TextureLayout::DepthComponent: return GL_DEPTH_COMPONENT;
	case TextureLayout::StencilIndex: return GL_STENCIL_INDEX;
	default:
		assert(false);
	}

	return 0;
}

static GLenum translateExternalType(BufferDataType type)
{
	switch (type)
	{
	case BufferDataType::Byte:		return GL_BYTE;
	case BufferDataType::UByte:		return GL_UNSIGNED_BYTE;
	case BufferDataType::Short:		return GL_SHORT;
	case BufferDataType::UShort:	return GL_UNSIGNED_SHORT;
	case BufferDataType::Int:		return GL_INT;
	case BufferDataType::UInt:		return GL_UNSIGNED_INT;
	case BufferDataType::HalfFloat:	return GL_HALF_FLOAT;
	case BufferDataType::Float:		return GL_FLOAT;
	case BufferDataType::Double:	return GL_DOUBLE;
	case BufferDataType::Fixed:		return GL_FIXED;
	default:
		assert(false);
	}

	return 0;
}

void GlTexture::SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data)
{
	const auto externalFormat = translateExternalFormat(dataFormat.ChannelsLayout);
	const auto externalType = translateExternalType(dataFormat.DataType);

	std::visit(Utils::overloaded {
 [=](const Texture1D& type)
		{
			glTextureSubImage1DEXT(m_id, GL_TEXTURE_1D, level, m_internalFormat, size, externalFormat, externalType, data);
		},
 [=](auto&& type) //do nothing
		{
		}
		}, GetType());
}

void GlTexture::SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data)
{
	const auto externalFormat = translateExternalFormat(dataFormat.ChannelsLayout);
	const auto externalType = translateExternalType(dataFormat.DataType);

	std::visit(Utils::overloaded{
 [=](const Texture2D& type)
		{
			glTextureSubImage2DEXT(m_id, GL_TEXTURE_2D, level, 0, 0, size.x, size.y, externalFormat, externalType, data);
		},
 [=](auto&& type) //do nothing
		{
			throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "Probably not implemented");
;		}
		}, GetType());
}

void GlTexture::SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data)
{
	const auto externalFormat = translateExternalFormat(dataFormat.ChannelsLayout);
	const auto externalType = translateExternalType(dataFormat.DataType);

	std::visit(Utils::overloaded{
 [=](const Texture3D& type)
		{
			glTextureSubImage3DEXT(m_id, GL_TEXTURE_3D, level, offset.x, offset.y, offset.z, size.x, size.y, size.z, externalFormat, externalType, data);
		},
 [=](auto&& type) //do nothing
		{
	 throw AT2Exception(AT2Exception::ErrorCase::NotImplemented, "Probably not implemented");
		}
		}, GetType());
}


