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
		GlTexture(TextureType type, GLint numComponents, GLenum format);
		virtual ~GlTexture();

		virtual void UpdateData(GLenum target, BufferData data) = 0;
		virtual void Bind(unsigned int module);
		virtual void Unbind();
		virtual void BuildMipmaps();

		virtual int GetCurrentModule() const {return m_currentTextureModule;}
		virtual unsigned int GetId() const {return m_id;}

	protected:
		TextureType m_targetType;
		GLuint m_id;
		int m_currentTextureModule;

		GLint m_numComponents;
		GLenum m_format;
	};

	class GlTexture1D : public GlTexture
	{
	public:
		GlTexture1D(GLint numComponents, GLenum format);
		virtual void UpdateData(GLenum target, BufferData data);
	};

	class GlTexture2D : public GlTexture
	{
	public:
		GlTexture2D(GLint numComponents, GLenum format);
		virtual void UpdateData(GLenum target, BufferData data);
	};

	class GlTexture3D : public GlTexture
	{
	public:
		GlTexture3D(GLint numComponents, GLenum format);
		virtual void UpdateData(GLenum target, BufferData data);
	};
}

#endif