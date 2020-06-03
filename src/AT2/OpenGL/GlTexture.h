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
			Texture1DArray = GL_TEXTURE_1D_ARRAY,

			Texture2D = GL_TEXTURE_2D,
			Texture2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
			Texture2DRectangle = GL_TEXTURE_RECTANGLE,
			Texture2DArray = GL_TEXTURE_2D_ARRAY,
			Texture2DMultisampleArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,

			Texture3D = GL_TEXTURE_3D,

			CubeMap = GL_TEXTURE_CUBE_MAP,
			CubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY
		};

		struct BufferData
		{
			unsigned int Height, Width, Depth;
			GLenum ExternalFormat, DataType;
			void* Data;

			BufferData() : Height(0), Width(0), Depth(0), ExternalFormat(0), DataType(0), Data(nullptr) {}
		};

		GlTexture(TextureType type, GLint internalFormat);
		GlTexture(GLuint id, TextureType type, GLint internalFormat);
		GlTexture(TextureType type, GLint internalFormat, GLenum format, GLenum dataType);

		virtual ~GlTexture();

	public:
		void Bind(unsigned int module) override;
		void Unbind() override;
		void BuildMipmaps() override;
		glm::uvec3 GetSize() const override		{ return m_size; }

		int GetCurrentModule() const override	{return m_currentTextureModule;}
		unsigned int GetId() const override		{return m_id;}

		TextureType GetTargetType() const		{return m_targetType; }

	protected:
		void DetermineExternalFormatAndDataType();

	protected:
		TextureType m_targetType;
		GLuint m_id;
		int m_currentTextureModule;

		GLint m_internalFormat;
		GLenum m_format, m_dataType;

		glm::uvec3 m_size;
	};

	class GlTexture1D : public GlTexture
	{
	public:
		GlTexture1D(GLint internalFormat, GLuint size, GLuint levels=1);
		
	public:
		void SetData(GLuint level, BufferData data); //TODO!!! external format - характеристика входных данных! Передавать надо в SetData!
	};


	class GlTexture2D : public GlTexture
	{
	public:
		GlTexture2D(GLint internalFormat, glm::uvec2 size, GLuint levels=1);
		GlTexture2D(GLuint id, GLint internalFormat, glm::uvec2 size, GLuint levels = 1); //TODO временный костыль для LibOVR

	public:
		void SetData(GLuint level, BufferData data);
		void CopyFromFramebuffer(GLuint _level, glm::ivec2 pos, glm::uvec2 size);
	};


	class GlTexture2DArray : public GlTexture
	{
	public:
		GlTexture2DArray(GLint internalFormat, glm::uvec3 size, GLuint levels = 1);

	public:
		void SetLayer(GLuint level, GLuint layer, BufferData data);
	};


	class GlTexture3D : public GlTexture
	{
	public:
		GlTexture3D(GLint internalFormat, glm::uvec3 size, GLuint levels = 1);

	public:
		void SetLayer(GLuint level, GLuint layer, BufferData data);
		void SetData(GLuint level, BufferData data);
	};


	class GlTextureCube : public GlTexture
	{
	public:
		enum class CubeMapFace
		{
			PositiveX = 0,
			NegativeX,
			PositiveY,
			NegativeY,
			PositiveZ,
			NegativeZ
		};


		GlTextureCube(GLint internalFormat, glm::uvec2 size, GLuint levels = 1);

	public:
		void SetFace(CubeMapFace face, GLuint level, BufferData data);
	};
}

#endif