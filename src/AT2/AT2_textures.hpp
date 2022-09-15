#pragma once

#include <variant>
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>

namespace AT2
{

enum class TextureFormat
{
    R8Snorm,
    R8Unorm,
    R16Snorm,
    R16Unorm,
    R16Sint,
    R16Uint,
    R32Sint,
    R32Uint,
    R16Float,
    R32Float,

    RG8Snorm,
    RG8Unorm,
    RG16Snorm,
    RG16Unorm,
    RG16Sint,
    RG16Uint,
    RG32Sint,
    RG32Uint,
    RG16Float,
    RG32Float,

    RGBA8Snorm,
    RGBA8Unorm,
    RGBA8Unorm_sRGB,
    BGRA8Unorm,
    BGRA8Unorm_sRGB,
    RGBA16Snorm,
    RGBA16Unorm,
    RGBA8Sint,
    RGBA8Uint,
    RGBA16Sint,
    RGBA16Uint,
    RGBA32Sint,
    RGBA32Uint,
    RGBA16Float,
    RGBA32Float,

    Depth16Unorm,
    Depth32Float,
    Depth24Unorm_Stencil8,
    Depth32Float_Stencil8,
};

template <typename Base>
class MipmappingMixin : public Base
{
    unsigned m_levels = 1;

public:
    template<typename ... Args>
    constexpr MipmappingMixin(unsigned levels, Args... args) :
        Base(args...), m_levels(levels) {}

    [[nodiscard]] constexpr unsigned getLevels() const noexcept { return m_levels; }
};

template <typename Base>
class MultisampleMixin : public Base
{
    glm::u8 m_samples = 1;
    bool m_fixedSampleLocations = false;

public:
    template <typename... Args>
    constexpr MultisampleMixin(glm::u8 samples, bool fixedSampleLocations, Args... args) : Base(args...), m_samples(samples), m_fixedSampleLocations(fixedSampleLocations)
    {
    }

    [[nodiscard]] constexpr unsigned getSamples() const noexcept { return m_samples; }
    [[nodiscard]] constexpr bool getFixedSampleLocations() const noexcept { return m_fixedSampleLocations; }
};

template <size_t N>
requires (N <= 4)
class TextureBase
{
public:
    using size_vec = glm::vec<N, glm::u32>;
    constexpr TextureBase(TextureFormat format, size_vec size) : size(size), format(format) {}

    [[nodiscard]] constexpr size_vec getSize() const noexcept { return size; }
    [[nodiscard]] constexpr TextureFormat getFormat() const noexcept { return format; }

private:
    size_vec size;
    TextureFormat format;
};


struct Texture1D : MipmappingMixin<TextureBase<1>>
{
    Texture1D(TextureFormat format, glm::uvec1 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<1>>(levels, format, size) {}
};

struct Texture1DArray : MipmappingMixin<TextureBase<2>>
{
    Texture1DArray(TextureFormat format, glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, format, size) {}
};

struct Texture2D : MipmappingMixin<TextureBase<2>>
{
    Texture2D(TextureFormat format, glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, format, size) {}
};

struct Texture2DMultisample : MultisampleMixin<TextureBase<2>>
{
    Texture2DMultisample(TextureFormat format, glm::uvec2 size, glm::u8 samples, bool fixedSampleLocations) : MultisampleMixin<TextureBase<2>>(samples, fixedSampleLocations, format, size) {}
};

struct Texture2DRectangle : MipmappingMixin<TextureBase<2>>
{
    Texture2DRectangle(TextureFormat format, glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, format, size) {}
};

struct Texture2DArray : MipmappingMixin<TextureBase<3>>
{
    Texture2DArray(TextureFormat format, glm::uvec3 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<3>>(levels, format, size) {}
};

struct Texture2DMultisampleArray : MultisampleMixin<TextureBase<3>>
{
    Texture2DMultisampleArray(TextureFormat format, glm::uvec3 size, glm::u8 samples, bool fixedSampleLocations = true) : MultisampleMixin<TextureBase<3>>(samples, fixedSampleLocations, format, size) {}
};

struct TextureCube : MipmappingMixin<TextureBase<2>>
{
    TextureCube(TextureFormat format, glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, format, size) {}
};

struct TextureCubeArray : MipmappingMixin<TextureBase<3>>
{
    TextureCubeArray(TextureFormat format, glm::uvec3 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<3>>(levels, format, size) {}
};

struct Texture3D : MipmappingMixin<TextureBase<3>>
{
    Texture3D(TextureFormat format, glm::uvec3 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<3>>(levels, format, size){}
};

using Texture = std::variant<
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DMultisample,
    Texture2DRectangle,//TODO: remove, deduce type automatically by dimensions
    Texture2DArray,
    Texture2DMultisampleArray,
    TextureCube,
    TextureCubeArray,
    Texture3D
>; 

constexpr inline TextureFormat getTextureFormat(const Texture& textureDesc)
{
    return std::visit([](const auto& concreteDesc){return concreteDesc.getFormat();}, textureDesc );
}

constexpr inline size_t getPixelSize(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::R8Snorm:
    case TextureFormat::R8Unorm:
        return 1;

    case TextureFormat::R16Snorm:
    case TextureFormat::R16Unorm:
    case TextureFormat::R16Sint:
    case TextureFormat::R16Uint:
    case TextureFormat::R16Float:
    case TextureFormat::RG8Snorm:
    case TextureFormat::RG8Unorm:
    case TextureFormat::Depth16Unorm:
        return 2;

    case TextureFormat::R32Sint:
    case TextureFormat::R32Uint:
    case TextureFormat::R32Float:
    case TextureFormat::RG16Snorm:
    case TextureFormat::RG16Unorm:
    case TextureFormat::RG16Sint:
    case TextureFormat::RG16Uint:
    case TextureFormat::RG16Float:
    case TextureFormat::RGBA8Snorm:
    case TextureFormat::RGBA8Unorm:
    case TextureFormat::RGBA8Unorm_sRGB:
    case TextureFormat::BGRA8Unorm:
    case TextureFormat::BGRA8Unorm_sRGB:
    case TextureFormat::RGBA8Sint:
    case TextureFormat::RGBA8Uint:
    case TextureFormat::Depth24Unorm_Stencil8:
    case TextureFormat::Depth32Float:
        return 4;

    case TextureFormat::RG32Sint:
    case TextureFormat::RG32Uint:
    case TextureFormat::RG32Float:
    case TextureFormat::RGBA16Snorm:
    case TextureFormat::RGBA16Unorm:
    case TextureFormat::RGBA16Sint:
    case TextureFormat::RGBA16Uint:
    case TextureFormat::RGBA16Float:
    case TextureFormat::Depth32Float_Stencil8:
        return 8;

    case TextureFormat::RGBA32Sint:
    case TextureFormat::RGBA32Uint:
    case TextureFormat::RGBA32Float: 
        return 16;
    }

    assert(false);
    return 0;
}

}