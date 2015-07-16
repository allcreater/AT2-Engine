#ifndef AT2_TYPES_H
#define AT2_TYPES_H

#include <glm/glm.hpp>

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
};

};

#endif