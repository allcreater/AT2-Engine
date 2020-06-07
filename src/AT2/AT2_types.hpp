#ifndef AT2_TYPES_H
#define AT2_TYPES_H

#include <glm/glm.hpp>
#include "AABB.h"

namespace AT2
{
	
enum class BufferDataType : unsigned char
{
	Byte,
	UByte,
	Short,
	UShort,
	Int,
	UInt,
	HalfFloat,
	Float,
	Double,
	Fixed
};

enum class VertexBufferType : unsigned char//TODO: Not full list of types!
{
	ArrayBuffer,
	IndexBuffer,
	UniformBuffer
};

struct BufferTypeInfo
{
	//const char* Name;
	BufferDataType Type;
	unsigned char Count;
	unsigned int Stride;
	bool IsNormalized;

	BufferTypeInfo() = default;
	BufferTypeInfo(BufferDataType type, unsigned char count, unsigned int stride, bool isNormalized = false) : Type(type), Count(count), Stride(stride), IsNormalized(isNormalized) {}
};


namespace BufferDataTypes
{
	template<typename T> BufferTypeInfo MakeBufferTypeInfo(BufferDataType type, unsigned char count, bool isNormalized = false)
	{
		return BufferTypeInfo(type, count, sizeof(T), isNormalized);
	}

	//scalars
	const BufferTypeInfo Byte = BufferTypeInfo(BufferDataType::Byte, 1, 1);
	const BufferTypeInfo UByte = BufferTypeInfo(BufferDataType::UByte, 1, 1);

	const BufferTypeInfo Short = BufferTypeInfo(BufferDataType::Short, 1, 2);
	const BufferTypeInfo UShort = BufferTypeInfo(BufferDataType::UShort, 1, 2);

	const BufferTypeInfo Int = BufferTypeInfo(BufferDataType::Int, 1, 4);
	const BufferTypeInfo UInt = BufferTypeInfo(BufferDataType::UInt, 1, 4);

	const BufferTypeInfo Float = BufferTypeInfo(BufferDataType::Float, 1, 4);
	const BufferTypeInfo Double = BufferTypeInfo(BufferDataType::Double, 1, 8);

	//vectors
	const BufferTypeInfo BVec2 = MakeBufferTypeInfo<glm::bvec2>(BufferDataType::Byte, 2);
	const BufferTypeInfo BVec3 = MakeBufferTypeInfo<glm::bvec3>(BufferDataType::Byte, 3);
	const BufferTypeInfo BVec4 = MakeBufferTypeInfo<glm::bvec4>(BufferDataType::Byte, 4);

	const BufferTypeInfo IVec2 = MakeBufferTypeInfo<glm::ivec2>(BufferDataType::Int, 2);
	const BufferTypeInfo IVec3 = MakeBufferTypeInfo<glm::ivec3>(BufferDataType::Int, 3);
	const BufferTypeInfo IVec4 = MakeBufferTypeInfo<glm::ivec4>(BufferDataType::Int, 4);

	const BufferTypeInfo Vec2 = MakeBufferTypeInfo<glm::vec2>(BufferDataType::Float, 2);
	const BufferTypeInfo Vec3 = MakeBufferTypeInfo<glm::vec3>(BufferDataType::Float, 3);
	const BufferTypeInfo Vec4 = MakeBufferTypeInfo<glm::vec4>(BufferDataType::Float, 4);

	const BufferTypeInfo DVec2 = MakeBufferTypeInfo<glm::dvec2>(BufferDataType::Double, 2);
	const BufferTypeInfo DVec3 = MakeBufferTypeInfo<glm::dvec3>(BufferDataType::Double, 3);
	const BufferTypeInfo DVec4 = MakeBufferTypeInfo<glm::dvec4>(BufferDataType::Double, 4);
}


/// <summary>
/// Texture input data layout
/// </summary>

//subset from https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexSubImage2D.xhtml
enum class TextureLayout
{
	//Index,
	Red,
	//Green,
	//Blue,
	//Alpha,
	RG,
	RGB,
	BGR,
	RGBA,
	BGRA,

	//Luminance,
	//LuminanceAlpha
	DepthComponent,
	StencilIndex
};

struct ExternalTextureFormat
{
	TextureLayout ChannelsLayout;
	BufferDataType DataType;
};

}

#endif