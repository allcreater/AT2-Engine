#include <filesystem>

#include "GlPipelineState.h"
#include "GlRenderer.h"
#include "GlShaderProgram.h"
#include "GlTexture.h"
#include "GlVertexArray.h"

#include <utils.hpp>

#include <fstream>
#include <optional>
#include "GlFrameBuffer.h"

using namespace std::literals;
using namespace AT2;
using namespace OpenGL41;

namespace
{
    constexpr GLint DetermineInternalFormat(ExternalTextureFormat format)
    {
        if (format.DataType == BufferDataType::Double || format.DataType == BufferDataType::Fixed)
            throw AT2NotImplementedException("double and fixed-point buffer layout support not implemented");

        switch (format.ChannelsLayout)
        {
        case TextureLayout::Red:
            switch (format.DataType)
            {
            case BufferDataType::Byte:
            case BufferDataType::UByte: return GL_R8;
            case BufferDataType::Short:
            case BufferDataType::UShort: return GL_R16;
            case BufferDataType::Int:
            case BufferDataType::UInt: return GL_R32I; //TODO: check correctness
            case BufferDataType::HalfFloat: return GL_R16F;
            case BufferDataType::Float: return GL_R32F;
            default: throw AT2Exception("Unsupported external format DataType");
            }

        case TextureLayout::RG:
            switch (format.DataType)
            {
            case BufferDataType::Byte:
            case BufferDataType::UByte: return GL_RG8;
            case BufferDataType::Short:
            case BufferDataType::UShort: return GL_RG16;
            case BufferDataType::Int:
            case BufferDataType::UInt: return GL_RG32I; //TODO: check correctness
            case BufferDataType::HalfFloat: return GL_RG16F;
            case BufferDataType::Float: return GL_RG32F;
            default: throw AT2Exception("Unsupported external format DataType");
            }

        case TextureLayout::RGB:
            switch (format.DataType)
            {
            case BufferDataType::Byte:
            case BufferDataType::UByte: return format.PreferSRGB ? GL_SRGB8 : GL_RGB8;
            case BufferDataType::Short:
            case BufferDataType::UShort: return GL_RGB16;
            case BufferDataType::Int:
            case BufferDataType::UInt: return GL_RGB32I; //TODO: check correctness
            case BufferDataType::HalfFloat: return GL_RGB16F;
            case BufferDataType::Float: return GL_RGB32F;
            default: throw AT2Exception("Unsupported external format DataType");
            }

        case TextureLayout::RGBA:
            switch (format.DataType)
            {
            case BufferDataType::Byte:
            case BufferDataType::UByte: return format.PreferSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            case BufferDataType::Short:
            case BufferDataType::UShort: return GL_RGBA16;
            case BufferDataType::Int:
            case BufferDataType::UInt: return GL_RGBA32I; //TODO: check correctness
            case BufferDataType::HalfFloat: return GL_RGBA16F;
            case BufferDataType::Float: return GL_RGBA32F;
            default: throw AT2Exception("Unsupported external format DataType");
            }


        case TextureLayout::DepthComponent:
            switch (format.DataType)
            {
            case BufferDataType::Byte:
            case BufferDataType::UByte: throw AT2NotImplementedException("Depth component buffer does not supports byte formats");
            case BufferDataType::Short:
            case BufferDataType::UShort:
            case BufferDataType::HalfFloat: return GL_DEPTH_COMPONENT16;
            case BufferDataType::Int:
            case BufferDataType::UInt: return GL_DEPTH_COMPONENT32; //TODO: check correctness
            case BufferDataType::Float: return GL_DEPTH_COMPONENT32F;
            default: throw AT2Exception("Unsupported external format DataType");
            }
        case TextureLayout::DepthStencil:
            switch (format.DataType)
            {
            case BufferDataType::UInt: return GL_DEPTH24_STENCIL8;
            case BufferDataType::Float: return GL_DEPTH32F_STENCIL8;
            default: throw AT2Exception("Depth stencil texture could be UInt(for 24bit depth + 8 stencil) or Float (32 + 8)");
            }
        default: throw AT2Exception("Unsupported external format ChannelsLayout");
        }
    }
} // namespace


GlResourceFactory::GlResourceFactory(GlRenderer& renderer) : m_renderer(renderer) {}

std::shared_ptr<ITexture> GlResourceFactory::CreateTextureFromFramebuffer(const glm::ivec2& pos,
                                                                          const glm::uvec2& size) const
{
    auto texture = GlTexture::Make(m_renderer, Texture2D {size},
                                               DetermineInternalFormat(TextureFormats::RGBA8), TextureFormats::RGBA8); //TODO: choose formats?
    texture->CopyFromFramebuffer(0, pos, size, {});
    return texture;
}

std::shared_ptr<ITexture> GlResourceFactory::CreateTexture(const Texture& declaration,
                                                           ExternalTextureFormat desiredFormat) const
{
    return GlTexture::Make(m_renderer, declaration, DetermineInternalFormat(desiredFormat), desiredFormat);
}

std::shared_ptr<IFrameBuffer> GlResourceFactory::CreateFrameBuffer() const
{
    return std::make_shared<GlFrameBuffer>(m_renderer);
}

std::shared_ptr<IVertexArray> GlResourceFactory::CreateVertexArray() const
{
    return std::make_shared<GlVertexArray>(m_renderer.GetRendererCapabilities());
}

std::shared_ptr<IBuffer> GlResourceFactory::CreateBuffer(VertexBufferType type) const
{
    return std::make_shared<GlBuffer>(type);
}

std::shared_ptr<IBuffer> GlResourceFactory::CreateBuffer(VertexBufferType type,
                                                                             std::span<const std::byte> data) const
{
    auto buffer = CreateBuffer(type);
    buffer->SetDataRaw(data);
    return buffer;
}

//TODO: Resource system!
std::shared_ptr<IShaderProgram> GlResourceFactory::CreateShaderProgramFromFiles(std::initializer_list<str> files) const
{
    class GlShaderProgramFromFileImpl : public IReloadable
    {
    public:
        GlShaderProgramFromFileImpl(GlRenderer& renderer, std::initializer_list<str> filenames) : m_renderer {renderer},
        m_filenames {ClassifyFilenames(filenames)}, m_shader {m_renderer, MakeShaderDescriptor(m_filenames)}
        {
        }

        void Reload() override { m_shader = GlShaderProgram {m_renderer, MakeShaderDescriptor(m_filenames)}; }

        ReloadableGroup getReloadableClass() const override { return ReloadableGroup::Shaders; }

        GlShaderProgram& GetShader() { return m_shader; }

    private:
        using ShaderType = GlShaderProgram::ShaderType;
        using ClassifiedFilenameList = std::vector<std::pair<std::string, ShaderType>>;

        static std::string LoadFile(std::filesystem::path _filename)
        {
            if (_filename.empty())
                return "";

            std::ifstream t(_filename);
            if (!t.is_open())
                throw AT2IOException(Utils::ConcatStrings("file '"sv, _filename.string(), "' not found."sv));


            return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        }

        static ShaderType GetShaderTypeFromExtension(std::string_view filename)
        {
            using namespace std::string_literals;
            using ExtTypePair = std::pair<std::string_view, ShaderType>;
            static constexpr std::array knownExtensions {ExtTypePair {".vs.glsl"sv, ShaderType::Vertex},
                                                         ExtTypePair {".tcs.glsl"sv, ShaderType::TesselationControl},
                                                         ExtTypePair {".tes.glsl"sv, ShaderType::TesselationEvaluation},
                                                         ExtTypePair {".gs.glsl"sv, ShaderType::Geometry},
                                                         ExtTypePair {".fs.glsl"sv, ShaderType::Fragment},
                                                         ExtTypePair {".cs.glsl"sv, ShaderType::Computational}};

            auto it = std::find_if(std::begin(knownExtensions), std::end(knownExtensions),
                                   [filename](const ExtTypePair& pair) { return filename.ends_with(pair.first); });
            return it != knownExtensions.end() ? it->second : throw AT2Exception(Utils::ConcatStrings("Couldn't deduce shader type from filename: "sv, filename));
        }

        static ClassifiedFilenameList ClassifyFilenames(std::initializer_list<str> filenames)
        {
            ClassifiedFilenameList classifiedFilenames {filenames.size()};
            std::transform(filenames.begin(), filenames.end(), classifiedFilenames.begin(), [](const std::string& filename) {
                return ClassifiedFilenameList::value_type {filename, GetShaderTypeFromExtension(filename)};
            });

            return classifiedFilenames;
        }

        static GlShaderProgram::ShaderDescriptor MakeShaderDescriptor(const ClassifiedFilenameList& filesList)
        {
            GlShaderProgram::ShaderDescriptor descriptor;
            std::transform(filesList.begin(), filesList.end(), std::inserter(descriptor, descriptor.end()),
                           [](ClassifiedFilenameList::const_reference classifiedPath) {
                               return GlShaderProgram::ShaderDescriptor::value_type {classifiedPath.second, LoadFile(classifiedPath.first)};
                           });
            return descriptor;
        }

    private:
        GlRenderer& m_renderer;
        ClassifiedFilenameList m_filenames;
        GlShaderProgram m_shader;
    };

    auto resource = std::make_shared<GlShaderProgramFromFileImpl>(m_renderer, files);
    m_reloadableResourcesList.push_back(std::weak_ptr<IReloadable>(resource));

    return {resource, &resource->GetShader()};
}

std::shared_ptr<IPipelineState> GlResourceFactory::CreatePipelineState(const PipelineStateDescriptor& pipelineStateDescriptor) const
{
    return std::make_shared<PipelineState>(m_renderer, pipelineStateDescriptor);
}

void GlResourceFactory::ReloadResources(ReloadableGroup group)
{
    for (const auto& resource : m_reloadableResourcesList)
    {
        if (auto reloadable = resource.lock())
            if (reloadable->getReloadableClass() == group)
                reloadable->Reload();
    }
}
