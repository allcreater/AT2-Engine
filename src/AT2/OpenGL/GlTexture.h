#ifndef AT2_GL_TEXTURE_H
#define AT2_GL_TEXTURE_H

#include "AT2lowlevel.h"
#include "../AT2_textures.hpp"

namespace AT2
{
	class GlTexture : public ITexture
	{
	public:
		GlTexture(Texture flavor, GLint desiredFormat);
		~GlTexture() override;

		void Bind(unsigned int module) override;
		void Unbind() override;
		void BuildMipmaps() override;
		glm::uvec3 GetSize() const noexcept override { return m_size; }

		int GetCurrentModule() const noexcept override { return m_currentTextureModule; }
		unsigned int GetId() const noexcept override { return m_id; }

		const Texture& GetType() const noexcept override { return m_flavor; }
        void SetWrapMode(TextureWrapMode wrapMode) override;
		const TextureWrapMode& GetWrapMode() const override { return m_wrapMode; }

		void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) override;
		void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data, int cubeMapFace = 0) override;
		void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) override;

		void CopyFromFramebuffer(GLuint _level, glm::ivec2 pos, glm::uvec2 size, int cubeMapFace = 0);

		GLenum GetTarget() const;

	protected:
		Texture m_flavor;
		TextureWrapMode m_wrapMode;

		GLuint m_id{ 0 };
		int m_currentTextureModule{ -1 };

		GLint m_internalFormat{ 0 };
		glm::uvec3 m_size;
		//GLenum m_format {0}, m_dataType {0};
	};

}

#endif