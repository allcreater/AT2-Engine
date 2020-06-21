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
		unsigned int Offset;
		bool IsNormalized;

		BufferTypeInfo() = default;
		BufferTypeInfo(BufferDataType type, unsigned char count, unsigned int stride, unsigned int offset = 0, bool isNormalized = false) : Type(type), Count(count), Stride(stride), Offset(offset), IsNormalized(isNormalized) {}
	};


	namespace BufferDataTypes
	{
		template<typename T> BufferTypeInfo MakeBufferTypeInfo(BufferDataType type, unsigned char count, bool isNormalized = false)
		{
			return BufferTypeInfo(type, count, sizeof(T), 0, isNormalized);
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

	enum class TextureWrapMode
	{
		ClampToEdge,
		ClampToBorder,
		MirroredRepeat,
		Repeat,
		MirrorClampToEdge
	};

    enum class BufferUsage
    {
        ReadOnly,
        ReadWrite,
        WriteOnly
    };

    namespace TextureFormats
    {
        constexpr auto RGBA8 = ExternalTextureFormat{ TextureLayout::RGBA, BufferDataType::UByte };
        constexpr auto RGBA16F = ExternalTextureFormat{ TextureLayout::RGBA, BufferDataType::HalfFloat };
        constexpr auto RGBA32F = ExternalTextureFormat{ TextureLayout::RGBA, BufferDataType::Float };
        constexpr auto DEPTH32F = ExternalTextureFormat{ TextureLayout::DepthComponent, BufferDataType::Float };
    }

    namespace Primitives
    {
        struct Points {};
        struct LineStrip {};
        struct LineLoop {};
        struct Lines {};
        struct LineStripAdjacency {};
        struct LinesAdjacency {};
        struct TriangleStrip {};
        struct TriangleFan {};
        struct Triangles {};
        struct TriangleStripAdjacency {};
        struct TrianglesAdjacency {};
        struct Patches { const int NumControlPoints = 3; };

        using Primitive = std::variant
            <
            Points,
            LineStrip,
            LineLoop,
            Lines,
            LineStripAdjacency,
            LinesAdjacency,
            TriangleStrip,
            TriangleFan,
            Triangles,
            TriangleStripAdjacency,
            TrianglesAdjacency,
            Patches
            >;
    }

}

using Uniform = std::variant
<
    int,
    glm::ivec2,
    glm::ivec3,
    glm::ivec4,

    unsigned int,
    glm::uvec2,
    glm::uvec3,
    glm::uvec4,

    float,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    glm::mat2,
    glm::mat3,
    glm::mat4,

    double,
    glm::dvec2,
    glm::dvec3,
    glm::dvec4,
    glm::dmat2,
    glm::dmat3,
    glm::dmat4
>;


#endif