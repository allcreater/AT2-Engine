#pragma once

#include <variant>
#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>

class BaseTexture
{
	glm::u8 levels = 1;
protected:
	//void checkSize(glm::uvec3 size) const
	//{
	//	const int maxLevels = glm::log2(glm::max(size.x, glm::max(size.y, size.z)));
	//	assert(levels <= maxLevels);
	//}

public:
	BaseTexture(unsigned int levels) : levels(levels) {}
	[[nodiscard]] unsigned int getLevels() const noexcept { return levels; }
};

class Texture1D : public BaseTexture
{
	glm::uvec1 size;
public:
	Texture1D(glm::u32 size, int levels = 1) : BaseTexture(levels), size(size) { }

	[[nodiscard]] glm::u32 getSize() const noexcept { return size.x; }
};

class Texture2D : public BaseTexture
{
	glm::uvec2 size;
public:
	Texture2D(glm::uvec2 size, int levels = 1) : BaseTexture(levels), size(size) {}

	[[nodiscard]] glm::uvec2 getSize() const noexcept { return size; }
};

class Texture2DArray : public BaseTexture
{
	glm::uvec3 size;
public:
	Texture2DArray(glm::uvec2 size, glm::u32 length, int levels = 1) : BaseTexture(levels), size({size, length}) {}

	[[nodiscard]] glm::uvec2 getSize() const noexcept { return glm::xy(size); }
	[[nodiscard]] glm::u32 getLength() const noexcept { return size.z; }
};

class TextureCube : public BaseTexture
{
	glm::uvec2 size;
public:
	TextureCube(glm::uvec2 size, int levels = 1) : BaseTexture(levels), size(size) {}

	[[nodiscard]] glm::uvec2 getSize() const noexcept { return size; }
};

class Texture3D : public BaseTexture
{
	glm::uvec3 size;
public:
	Texture3D(glm::uvec3 size, int levels = 1) : BaseTexture(levels), size(size) {}

	[[nodiscard]] glm::uvec3 getSize() const noexcept { return size; }
};

using Texture = std::variant<Texture1D, Texture2D, Texture2DArray, TextureCube, Texture3D>;
