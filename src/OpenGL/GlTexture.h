#ifndef AT2_GL_TEXTURE_H
#define AT2_GL_TEXTURE_H

#include "AT2lowlevel.h"

namespace AT2
{
	class GlTexture : public ITexture
	{
	public:
		enum class TextureType
		{
			Texture1D = GL_TEXTURE_1D,
			Texture2D = GL_TEXTURE_2D,
			Texture3D = GL_TEXTURE_3D,
			Texture1DArray = GL_TEXTURE_1D_ARRAY,
			Texture2DArray = GL_TEXTURE_2D_ARRAY,
			CubeMap = GL_TEXTURE_CUBE_MAP,
			CubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
			Rectangle = GL_TEXTURE_RECTANGLE
		};

	public:
		GlTexture(TextureType type, GLint internalFormat, GLenum format, GLenum dataType);
		virtual ~GlTexture();

		virtual void UpdateData(GLenum target, BufferData data) = 0;
		virtual void Bind(unsigned int module);
		virtual void Unbind();
		virtual void BuildMipmaps();

		virtual int GetCurrentModule() const {return m_currentTextureModule;}
		virtual unsigned int GetId() const {return m_id;}

		virtual TextureType GetTargetType() const {return m_targetType; }

	protected:
		TextureType m_targetType;
		GLuint m_id;
		int m_currentTextureModule;

		GLint m_internalFormat;
		GLenum m_format, m_dataType;
	};

	class GlTexture1D : public GlTexture
	{
	public:
		GlTexture1D(GLint internalFormat, GLenum format, GLenum dataType = GL_UNSIGNED_BYTE);
		virtual void UpdateData(GLenum target, BufferData data);
	};

	class GlTexture2D : public GlTexture
	{
	public:
		GlTexture2D(GLint internalFormat, GLenum format, GLenum dataType = GL_UNSIGNED_BYTE);
		virtual void UpdateData(GLenum target, BufferData data);
	};

	class GlTexture3D : public GlTexture
	{
	public:
		GlTexture3D(GLint internalFormat, GLenum format, GLenum dataType = GL_UNSIGNED_BYTE);
		virtual void UpdateData(GLenum target, BufferData data);
	};

	class GlTextureCube : public GlTexture
	{
	public:
		GlTextureCube(GLint internalFormat, GLenum format, GLenum dataType = GL_UNSIGNED_BYTE);
		virtual void UpdateData(GLenum target, BufferData data);
	};
}

#endif