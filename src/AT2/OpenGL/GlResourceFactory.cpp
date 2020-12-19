#include "GlRenderer.h"
#include "GlShaderProgram.h"
#include "GlTexture.h"
#include "GlVertexArray.h"

#include "../utils.hpp"

#include <fstream>
#include <optional>

#include "GlFrameBuffer.h"

using namespace std::literals;
using namespace AT2;

constexpr std::optional<GLint> DetermineInternalFormat(ExternalTextureFormat format)
{
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
        default: return {};
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
        default: return {};
        }

    case TextureLayout::RGB:
    case TextureLayout::BGR:
        switch (format.DataType)
        {
        case BufferDataType::Byte:
        case BufferDataType::UByte: return GL_RGB8;
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_RGB16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_RGB32I; //TODO: check correctness
        case BufferDataType::HalfFloat: return GL_RGB16F;
        case BufferDataType::Float: return GL_RGB32F;
        default: return {};
        }

    case TextureLayout::RGBA:
    case TextureLayout::BGRA:
        switch (format.DataType)
        {
        case BufferDataType::Byte:
        case BufferDataType::UByte: return GL_RGBA8;
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_RGBA16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_RGBA32I; //TODO: check correctness
        case BufferDataType::HalfFloat: return GL_RGBA16F;
        case BufferDataType::Float: return GL_RGBA32F;
        default: return {};
        }

    case TextureLayout::DepthComponent:
        switch (format.DataType)
        {
        case BufferDataType::Short:
        case BufferDataType::UShort: return GL_DEPTH_COMPONENT16;
        case BufferDataType::Int:
        case BufferDataType::UInt: return GL_DEPTH_COMPONENT32; //TODO: check correctness
        case BufferDataType::Float: return GL_DEPTH_COMPONENT32F;
        default: return {};
        }

    default: return {};
    }
}


GlResourceFactory::GlResourceFactory(GlRenderer* renderer) : m_renderer(renderer) {}

std::shared_ptr<ITexture> GlResourceFactory::CreateTextureFromFramebuffer(const glm::ivec2& pos,
                                                                          const glm::uvec2& size) const
{
    auto texture = std::make_shared<GlTexture>(Texture2D {size},
                                               *DetermineInternalFormat(TextureFormats::RGBA8)); //TODO: choose formats?
    texture->CopyFromFramebuffer(0, pos, size);
    return texture;
}

std::shared_ptr<ITexture> GlResourceFactory::CreateTexture(const Texture& declaration,
                                                           ExternalTextureFormat desiredFormat) const
{
    const auto internalFormat = DetermineInternalFormat(desiredFormat);
    if (!internalFormat)
        throw AT2Exception(AT2Exception::ErrorCase::Texture, "Unsupported texture format");

    return std::make_shared<GlTexture>(declaration, *internalFormat);
}

std::shared_ptr<IFrameBuffer> GlResourceFactory::CreateFrameBuffer() const
{
    return std::make_shared<GlFrameBuffer>(m_renderer->GetRendererCapabilities());
}

std::shared_ptr<IVertexArray> GlResourceFactory::CreateVertexArray() const
{
    return std::make_shared<GlVertexArray>(m_renderer->GetRendererCapabilities());
}

std::shared_ptr<IVertexBuffer> GlResourceFactory::CreateVertexBuffer(VertexBufferType type) const
{
    return std::make_shared<GlVertexBuffer>(type);
}

std::shared_ptr<IVertexBuffer> GlResourceFactory::CreateVertexBufferInternal(VertexBufferType type,
                                                                             std::span<const std::byte> data) const
{
    auto buffer = CreateVertexBuffer(type);
    buffer->SetDataRaw(data);
    return buffer;
}

//TODO: detach file as a shader source from specific implementation, remove inheritance
std::shared_ptr<IShaderProgram> GlResourceFactory::CreateShaderProgramFromFiles(std::initializer_list<str> files) const
{

    class GlShaderProgramFromFileImpl : public GlShaderProgram, public virtual IReloadable
    {
    public:
        GlShaderProgramFromFileImpl(std::initializer_list<str> _shaders) : GlShaderProgram()
        {
            for (const auto& filename : _shaders)
            {
                if (GetName().empty())
                    SetName(filename);

                if (filename.substr(filename.length() - 8) == ".vs.glsl")
                    m_filenames.emplace_back(filename, AT2::ShaderType::Vertex);
                else if (filename.substr(filename.length() - 9) == ".tcs.glsl")
                    m_filenames.emplace_back(filename, AT2::ShaderType::TesselationControl);
                else if (filename.substr(filename.length() - 9) == ".tes.glsl")
                    m_filenames.emplace_back(filename, AT2::ShaderType::TesselationEvaluation);
                else if (filename.substr(filename.length() - 8) == ".gs.glsl")
                    m_filenames.emplace_back(filename, AT2::ShaderType::Geometry);
                else if (filename.substr(filename.length() - 8) == ".fs.glsl")
                    m_filenames.emplace_back(filename, AT2::ShaderType::Fragment);
                else if (filename.substr(filename.length() - 8) == ".cs.glsl")
                    m_filenames.emplace_back(filename, AT2::ShaderType::Computational);
                else
                    throw AT2Exception(AT2Exception::ErrorCase::Shader, "unrecognized shader type"s);
            }

            Reload();
        }

        void Reload() override
        {
            CleanUp();

            for (const auto& shader : m_filenames)
            {
                GlShaderProgram::AttachShader(LoadShader(shader.first), shader.second);
            }
        }

        ReloadableGroup getReloadableClass() const override { return ReloadableGroup::Shaders; }

    private:
        std::string LoadShader(const str& _filename)
        {
            if (_filename.empty())
                return "";

            std::ifstream t(_filename);
            if (!t.is_open())
                throw AT2Exception(AT2Exception::ErrorCase::File, "file '"s + _filename + "' not found.");


            return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        }

    private:
        std::vector<std::pair<str, AT2::ShaderType>> m_filenames;
    };

    auto resource = std::make_shared<GlShaderProgramFromFileImpl>(files);
    m_reloadableResourcesList.push_back(std::weak_ptr<IReloadable>(resource));
    return resource;
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
