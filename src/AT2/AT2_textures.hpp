#pragma once

#include <variant>
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>

template <size_t N, typename = std::enable_if<N <= 4> >
class BaseTexture
{
public:
	using size_vec = glm::vec<N, glm::u32>;
	BaseTexture(size_vec size, unsigned int levels) : size(size), levels(levels) {}

	[[nodiscard]] unsigned int getLevels() const noexcept { return levels; }
	[[nodiscard]] size_vec getSize() const noexcept { return size; }

protected:
	//void checkSize() const
	//{
	//	const int maxLevels = log2(glm::max(size.x, glm::max(size.y, size.z)));
	//	assert(levels <= maxLevels);
	//}
	//
private:
	size_vec size;
	glm::u8 levels;
};

template <size_t N, typename = std::enable_if<N < 4> >
struct BaseTextureArray : BaseTexture<N+1>
{
	BaseTextureArray(typename BaseTexture<N + 1>::size_vec size, unsigned int levels) : BaseTexture(size, levels) {}
};

struct Texture1D : BaseTexture<1>
{
	Texture1D(glm::uvec1 size, unsigned int levels = 1) : BaseTexture(size, levels) { }
};

struct Texture1DArray : BaseTextureArray<1>
{
	Texture1DArray(glm::uvec2 size, unsigned int levels = 1) : BaseTextureArray(size, levels) { }
};

struct Texture2D : BaseTexture<2>
{
	Texture2D(glm::uvec2 size, unsigned int levels = 1) : BaseTexture(size, levels) { }
};

struct Texture2DArray : BaseTextureArray<2>
{
	Texture2DArray(glm::uvec3 size, unsigned int levels = 1) : BaseTextureArray(size, levels) { }
};

struct TextureCube : BaseTexture<2>
{
	TextureCube(glm::uvec2 size, unsigned int levels = 1) : BaseTexture(size, levels) { }
};

struct TextureCubeArray : BaseTextureArray<2>
{
	TextureCubeArray(glm::uvec3 size, unsigned int levels = 1) : BaseTextureArray(size, levels) { }
};

struct Texture3D : BaseTexture<3>
{
	Texture3D(glm::uvec3 size, unsigned int levels = 1) : BaseTexture(size, levels) { }
};

//TODO: support all texture types
using Texture = std::variant<Texture1D, Texture2D, Texture2DArray, TextureCube, Texture3D>; 
