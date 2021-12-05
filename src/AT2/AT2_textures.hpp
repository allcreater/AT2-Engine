#pragma once

#include <variant>
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>

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
    constexpr TextureBase(size_vec size) : size(size) {}

    [[nodiscard]] constexpr size_vec getSize() const noexcept { return size; }

private:
    size_vec size;
};


struct Texture1D : MipmappingMixin<TextureBase<1>>
{
    Texture1D(glm::uvec1 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<1>>(levels, size) {}
};

struct Texture1DArray : MipmappingMixin<TextureBase<2>>
{
    Texture1DArray(glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, size) {}
};

struct Texture2D : MipmappingMixin<TextureBase<2>>
{
    Texture2D(glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, size) {}
};

struct Texture2DMultisample : MultisampleMixin<TextureBase<2>>
{
    Texture2DMultisample(glm::uvec2 size, glm::u8 samples, bool fixedSampleLocations) : MultisampleMixin<TextureBase<2>>(samples, fixedSampleLocations, size) {}
};

struct Texture2DRectangle : MipmappingMixin<TextureBase<2>>
{
    Texture2DRectangle(glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, size) {}
};

struct Texture2DArray : MipmappingMixin<TextureBase<3>>
{
    Texture2DArray(glm::uvec3 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<3>>(levels, size) {}
};

struct Texture2DMultisampleArray : MultisampleMixin<TextureBase<3>>
{
    Texture2DMultisampleArray(glm::uvec3 size, glm::u8 samples, bool fixedSampleLocations = true) : MultisampleMixin<TextureBase<3>>(samples, fixedSampleLocations, size) {}
};

struct TextureCube : MipmappingMixin<TextureBase<2>>
{
    TextureCube(glm::uvec2 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<2>>(levels, size) {}
};

struct TextureCubeArray : MipmappingMixin<TextureBase<3>>
{
    TextureCubeArray(glm::uvec3 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<3>>(levels, size) {}
};

struct Texture3D : MipmappingMixin<TextureBase<3>>
{
    Texture3D(glm::uvec3 size, unsigned int levels = 1) : MipmappingMixin<TextureBase<3>>(levels, size){}
};

using Texture = std::variant<
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DMultisample,
    Texture2DRectangle,
    Texture2DArray,
    Texture2DMultisampleArray,
    TextureCube,
    TextureCubeArray,
    Texture3D
>; 
