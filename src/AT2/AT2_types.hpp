#ifndef AT2_TYPES_H
#define AT2_TYPES_H

#include <glm/glm.hpp>

#include <chrono>
#include <type_traits>
#include <span>
#include <variant>

#include "AABB.h"
#include "flags.hpp"

namespace AT2
{
    using Seconds = std::chrono::duration<double>;

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

    enum class VertexBufferFlags : std::uint8_t //TODO: rename to ResourceUsage or something like this
    {
        // usages
        Immutable       = 0,
        Dynamic         = 1 << 0,
        Stream          = 1 << 1,
        
        //StaticSize      = 1 << 2,
        
        // types hints (may be unused)
        ArrayBuffer     = 1 << 3,
        IndexBuffer     = 1 << 4,
        UniformBuffer   = 1 << 5,
        
    };
    using VertexBufferType = EnumFlags<VertexBufferFlags>;

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

    enum class VertexStepFunc : uint8_t
    {
        PerVertex,
        PerInstance
    };

    //TODO: rename to BufferBindingParams
    struct BufferBindingParams2 final
    {
        constexpr BufferBindingParams2() = default;
        constexpr BufferBindingParams2(unsigned int stride, VertexStepFunc stepFunc = VertexStepFunc::PerVertex, unsigned int stepRate = 0)
            : Enabled{true}, StepFunc{stepFunc}, Stride{stride}, StepRate{stepRate} {}

        bool Enabled = false; // intrusive version of optional<BufferBindingParams>
        VertexStepFunc StepFunc;
        unsigned int Stride;
        unsigned int StepRate;
    };

    //TODO: !!! use just enum VertexFormat instead of tuples {Type, Count, IsNormalized} because they are not independent !!! 
    //tuple is for compatibility only!

    struct VertexAttributeLayout final
    {
        constexpr VertexAttributeLayout() = default;
        constexpr VertexAttributeLayout(BufferDataType type, unsigned int count, bool normalized, unsigned int bufferIndex, unsigned int offset = 0)
        : Enabled{true}, Type{type}, Count{static_cast<uint8_t>(count)}, IsNormalized{normalized}, BufferIndex{bufferIndex}, Offset{offset} {assert(count >= 1 && count <= 4);}

        bool Enabled = false;
        BufferDataType Type;
        uint8_t Count;
        bool IsNormalized = false;
        unsigned int BufferIndex;
        unsigned int Offset;
    };

    //TODO: separate VertexArrayDescriptorBuilder? 
    class VertexArrayDescriptor final
    {
    public:
        // TODO: determine limit on runtime.
        constexpr static size_t MaxBuffers = 8;
        constexpr static size_t MaxAttributes = 16;

        constexpr VertexArrayDescriptor& SetBufferLayout(size_t i, BufferBindingParams2 layout)
        {
            m_bufferLayouts.at(i) = layout;
            return *this;
        }

        constexpr VertexArrayDescriptor& SetVertexAttributeLayout(size_t i, VertexAttributeLayout layout)
        {
            m_vertexAttributes.at(i) = layout;
            return *this;
        }

        [[nodiscard]] constexpr bool IsValid() const noexcept
        {
            return std::all_of(m_vertexAttributes.begin(), m_vertexAttributes.end(), [this](const VertexAttributeLayout& attribute){
                if (!attribute.Enabled)
                    return true;

                const auto index = attribute.BufferIndex;
                return index < m_bufferLayouts.size() && m_bufferLayouts[index].Enabled && attribute.Count >= 1 && attribute.Count <= 4;
            });
        }

        [[nodiscard]] constexpr std::span<const BufferBindingParams2> GetBufferLayouts() const noexcept { return m_bufferLayouts; }
        [[nodiscard]] constexpr std::span<const VertexAttributeLayout> GetVertexAttributeLayouts() const noexcept { return m_vertexAttributes; }

    private:
        std::array<BufferBindingParams2, MaxBuffers> m_bufferLayouts;
        std::array<VertexAttributeLayout, MaxAttributes> m_vertexAttributes;
    };


    //TODO: to separate header
    namespace BufferDataTypes
    {
        template <typename T>
        requires std::is_arithmetic_v<T>
        constexpr BufferDataType DeduceBufferDataType()
        {
            const bool isUnsigned = std::is_unsigned_v<T>;
            if constexpr (std::is_same_v<T, float>)
                return BufferDataType::Float;
            if constexpr (std::is_same_v<T, double>)
                return BufferDataType::Double;

            switch (sizeof(T))
            {
            case 1: return isUnsigned ? BufferDataType::UByte : BufferDataType::Byte;
            case 2: return isUnsigned ? BufferDataType::UShort : BufferDataType::Short;
            case 4: return isUnsigned ? BufferDataType::UInt : BufferDataType::Int;
            default: break;
            }

            return BufferDataType::Byte; //TODO: think what to do here :)
        }

        template <typename T>
        constexpr BufferBindingParams BufferTypeOf = {DeduceBufferDataType<T>(), 1, sizeof(T)};

        template <typename T, glm::length_t L, glm::qualifier Q>
        constexpr BufferBindingParams BufferTypeOf<glm::vec<L, T, Q>> = {DeduceBufferDataType<T>(), L, sizeof(glm::vec<L, T, Q>)};

    	template <typename T, size_t N>
        constexpr BufferBindingParams BufferTypeOf<T[N]> = {DeduceBufferDataType<T>(), N, sizeof(T[N])};

    	template <typename T, size_t N>
        constexpr BufferBindingParams BufferTypeOf<std::array<T, N>> = {DeduceBufferDataType<T>(), N, sizeof(std::array<T, N>)};

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
    enum class TextureLayout
    {
        Red,
        RG,
        RGB,
        RGBA,
        //Luminance,
        //LuminanceAlpha
        DepthComponent,
        DepthStencil
    };

    //pair TextureLayout + BufferDataType is bad abstraction(because of asymmetric formats like 10_10_10_2 and DepthStencil24_8) 
    //TODO: Better to make PixelFormat enum or something like this 
    struct ExternalTextureFormat
    {
        TextureLayout ChannelsLayout;
        BufferDataType DataType;
        //TODO: flags ? Also there should be usage options like for buffers
        bool RenderTarget = false;
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

    struct TextureWrapParams
    {
        TextureWrapMode WrapS = TextureWrapMode::Repeat;
        TextureWrapMode WrapT = TextureWrapMode::Repeat;
        TextureWrapMode WrapR = TextureWrapMode::Repeat;

        static constexpr TextureWrapParams Uniform(TextureWrapMode mode) { return {mode, mode, mode}; }
    };

    enum class TextureSamplingMode
    {
        // no interpolation between adjacent texels
        Nearest,
        // linear interpolation between adjacent texels
        Linear
    };

    enum class MipmapSamplingMode
    {
        // not use automatic mipmap selection
        Manual,
        // select nearest mipmap
        Nearest,
        // linear interpolation between two nearest mipmaps
        Linear
    };
    using TextureMinificationMode = std::tuple<TextureSamplingMode, MipmapSamplingMode>;

    struct TextureSamplingParams
    {
        TextureSamplingMode Magnification = TextureSamplingMode::Linear;
        TextureMinificationMode Minification = {TextureSamplingMode::Linear, MipmapSamplingMode::Linear};

        static constexpr TextureSamplingParams Uniform(TextureSamplingMode samplingMode, bool mipmapping = false)
        {
            if (mipmapping)
                return {samplingMode, {samplingMode, samplingMode == TextureSamplingMode::Nearest ? MipmapSamplingMode::Nearest : MipmapSamplingMode::Linear}};

            return {samplingMode, {samplingMode, MipmapSamplingMode::Manual}};
        }
    }; 

    enum class BufferOperationFlags : std::uint8_t
    {
        Read = 1 << 0,
        Write = 1 << 1,
        InvalidateRange = 1 << 2,
        ReadWrite = Read | Write
    };
    using BufferOperation = EnumFlags<BufferOperationFlags>;

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

    glm::uint,
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

template <typename... Variants>
using VariantOfSpans = std::variant<std::span<Variants>...>;

using UniformArray = VariantOfSpans
<
    int,
    glm::ivec2,
    glm::ivec3,
    glm::ivec4,

    glm::uint,
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

enum class CursorMode
{
    Normal,
    Hidden,
    Disabled
};

//TODO: move to separate header
class IWindow
{
public:
    class IWindowContext //TODO: technically something very near to graphics context, we still don't have separate abstraction for it
    {
    public:
        virtual ~IWindowContext() = default;

        virtual IWindow& getWindow() = 0;
        virtual const IWindow& getWindow() const = 0;
    };

public:
    virtual ~IWindow() = default;

    ///@thread_safety main thread
    virtual bool isKeyDown(int keyCode) const = 0;
    ///@thread_safety main thread
    virtual bool isMouseKeyDown(int button) const = 0;

    ///@thread_safety safe
    virtual IWindow& setCursorMode(CursorMode cursorMode) = 0;

    ///@thread_safety safe
    virtual IWindow& setLabel(std::string label) = 0;
    virtual const std::string& getLabel() const = 0;

    ///@thread_safety safe
    virtual IWindow& setSize(glm::ivec2 size) = 0;
    virtual glm::ivec2 getSize() const = 0;

    ///@thread_safety safe
    virtual void requestAttention() = 0;

    ///@thread_safety safe
    virtual IWindow& setVSyncInterval(int interval) = 0;

    ///@thread_safety safe
    virtual IWindow& setCloseFlag(bool flag) = 0;
    virtual bool getCloseFlag() const = 0;

private:
    virtual void setWindowContext(std::unique_ptr<IWindowContext> newWindowContext) = 0;
    //virtual IWindowContext* getWindowContext() = 0;
    //virtual const IWindowContext* getWindowContext() const = 0;
};

#endif
