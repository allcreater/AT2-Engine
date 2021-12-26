#pragma once

#include "../AT2_textures.hpp"
#include "AT2lowlevel.h"

namespace AT2::Metal
{
    class MtlTexture : public ITexture
    {
    public:
        NON_COPYABLE_OR_MOVABLE(MtlTexture)

        MtlTexture(Texture flavor);
        ~MtlTexture() override;

        void Bind(unsigned int unit) const override;
        void BindAsImage(unsigned int unit, glm::u32 level, glm::u32 layer, bool isLayered,
                         BufferUsage usage = BufferUsage::ReadWrite) const override;

        void Unbind() const override;
        void BuildMipmaps() override;
        glm::uvec3 GetSize() const noexcept override { return glm::uvec3(m_size); }
        size_t GetDataLength() const noexcept override { return m_dataSize; }

        int GetCurrentModule() const noexcept override { return 0; }
        unsigned int GetId() const noexcept override { return 0; }

        const Texture& GetType() const noexcept override { return m_flavor; }

        void SubImage1D(glm::u32 offset, glm::u32 size, glm::u32 level, ExternalTextureFormat dataFormat,
                        const void* data) override;
        void SubImage2D(glm::uvec2 offset, glm::uvec2 size, glm::u32 level, ExternalTextureFormat dataFormat,
                        const void* data) override;
        void SubImage3D(glm::uvec3 offset, glm::uvec3 size, glm::u32 level, ExternalTextureFormat dataFormat,
                        const void* data) override;

        void CopyFromFramebuffer(int _level, glm::ivec2 pos, glm::ivec2 size, glm::ivec3 textureOffset = {});

        // ISampler implementation:
        void SetWrapMode(TextureWrapParams wrapParams) override;
        const TextureWrapParams& GetWrapMode() const noexcept override { return m_wrapParams; }

        void SetSamplingMode(TextureSamplingParams samplingParams) override;
        [[nodiscard]] const TextureSamplingParams& GetSamplingParams() const noexcept override { return m_sampling_params; }

        void SetAnisotropy(float anisotropy) override;
        [[nodiscard]] float GetAnisotropy() const noexcept override;


    protected:
        Texture m_flavor;
        TextureWrapParams m_wrapParams;
        TextureSamplingParams m_sampling_params;
        float m_anisotropy = 1.0f;

        //internal size representation is int instead of uint, so that we need to do casting on request :(
        glm::ivec3 m_size;

        size_t m_dataSize {0};
    };

} // namespace AT2::Metal
