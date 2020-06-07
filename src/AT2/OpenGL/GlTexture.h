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
		~GlTexture();

		void Bind(unsigned int module) override;
		void Unbind() override;
		void BuildMipmaps() override;
		glm::uvec3 GetSize() const override;

		int GetCurrentModule() const override { return m_currentTextureModule; }
		unsigned int GetId() const override { return m_id; }

		const Texture& GetType() const override { return m_flavor; }
        void SetWrapMode(TextureWrapMode wrapMode) override;
		const TextureWrapMode& GetWrapMode() const override { return m_wrapMode; }

		void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) override;
		void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) override;
		void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat, void* data) override;


		GLenum GetTarget() const;

	protected:
		Texture m_flavor;
		TextureWrapMode m_wrapMode;

		GLuint m_id{ 0 };
		int m_currentTextureModule{ -1 };

		GLint m_internalFormat{ 0 };
		//GLenum m_format {0}, m_dataType {0};
	};

}

#endif