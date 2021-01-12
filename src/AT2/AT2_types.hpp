#ifndef AT2_TYPES_H
#define AT2_TYPES_H

#include <glm/glm.hpp>
#include <type_traits>
#include "AABB.h"

namespace AT2
{

    enum class ShaderType
    {
        Vertex = 0,
        TesselationControl = 1,
        TesselationEvaluation = 2,
        Geometry = 3,
        Fragment = 4,
        Computational = 5
    };

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

    struct BufferBindingParams
    {
        BufferDataType Type;
        unsigned char Count;
        // The distance between elements within the buffer
        unsigned int Stride;
        // The offset of the first element of the buffer
        unsigned int Offset = 0;
        bool IsNormalized = false;
        unsigned int Divisor = 0;
    };

    namespace BufferDataTypes
    {
        template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        constexpr BufferDataType DeduceBufferDataType()
        {
            const bool isUnsigned = std::is_unsigned_v<T>;
            if (std::is_same_v<T, float>)
                return BufferDataType::Float;
            if (std::is_same_v<T, double>)
                return BufferDataType::Double;

            static_assert(std::is_arithmetic_v<T>);

            switch (sizeof(T))
            {
            case 1: return isUnsigned ? BufferDataType::UByte : BufferDataType::Byte;
            case 2: return isUnsigned ? BufferDataType::UShort : BufferDataType::Short;
            case 4: return isUnsigned ? BufferDataType::UInt : BufferDataType::Int;
            default: break;
            }

            return BufferDataType::Byte; //TODO: think what to do here :)
        }

        template <typename T, typename = std::enable_if<std::is_arithmetic_v<T>>>
        constexpr BufferBindingParams BufferTypeOf = {DeduceBufferDataType<T>(), 1, sizeof(T)};

        template <typename T, glm::length_t L, glm::qualifier Q>
        constexpr BufferBindingParams BufferTypeOf<glm::vec<L, T, Q>> = {DeduceBufferDataType<T>(), L, sizeof(glm::vec<L, T, Q>)};

        //scalars
        constexpr BufferBindingParams Byte = BufferTypeOf<std::int8_t>;
        constexpr BufferBindingParams UByte = BufferTypeOf<std::uint8_t>;

        constexpr BufferBindingParams Short = BufferTypeOf<std::int16_t>;
        constexpr BufferBindingParams UShort = BufferTypeOf<std::uint16_t>;

        constexpr BufferBindingParams Int = BufferTypeOf<std::int32_t>;
        constexpr BufferBindingParams UInt = BufferTypeOf<std::uint32_t>;

        constexpr BufferBindingParams Float = BufferTypeOf<std::float_t>;
        constexpr BufferBindingParams Double = BufferTypeOf<std::double_t>;

        ////vectors
        constexpr BufferBindingParams BVec2 = BufferTypeOf<glm::bvec2>;
        constexpr BufferBindingParams BVec3 = BufferTypeOf<glm::bvec3>;
        constexpr BufferBindingParams BVec4 = BufferTypeOf<glm::bvec4>;

        constexpr BufferBindingParams IVec2 = BufferTypeOf<glm::ivec2>;
        constexpr BufferBindingParams IVec3 = BufferTypeOf<glm::ivec3>;
        constexpr BufferBindingParams IVec4 = BufferTypeOf<glm::ivec4>;

        constexpr BufferBindingParams Vec2 = BufferTypeOf<glm::vec2>;
        constexpr BufferBindingParams Vec3 = BufferTypeOf<glm::vec3>;
        constexpr BufferBindingParams Vec4 = BufferTypeOf<glm::vec4>;

        constexpr BufferBindingParams DVec2 = BufferTypeOf<glm::dvec2>;
        constexpr BufferBindingParams DVec3 = BufferTypeOf<glm::dvec3>;
        constexpr BufferBindingParams DVec4 = BufferTypeOf<glm::dvec4>;
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
        bool PreferSRGB = false;
    };

    enum class TextureWrapMode
    {
        ClampToEdge,
        ClampToBorder,
        MirroredRepeat,
        Repeat,
        MirrorClampToEdge
    };

    enum class BufferUsage : char
    {
        Read = 1 << 0,
        Write = 1 << 1,
        ReadWrite = Read | Write
    };

    inline BufferUsage operator|(BufferUsage lhs, BufferUsage rhs)
    {
        return static_cast<BufferUsage>(static_cast<char>(lhs) | static_cast<char>(rhs));
    }

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