#include "ResourceFactory.h"

#include "Renderer.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "ShaderProgram.h"
#include "Mappings.h"

#include <filesystem>
#include <fstream>

using namespace AT2;
using namespace AT2::Metal;

ResourceFactory::ResourceFactory(Renderer& renderer) : m_renderer(renderer)
{
	
}

std::shared_ptr<ITexture> ResourceFactory::CreateTextureFromFramebuffer(const glm::ivec2& pos, const glm::uvec2& size) const
{
    return std::make_shared<MtlTexture>(m_renderer, Texture2D{size}, MTL::PixelFormatRGBA8Unorm);
}

std::shared_ptr<ITexture> ResourceFactory::CreateTexture(const Texture& declaration, ExternalTextureFormat desiredFormat) const
{
    return std::make_shared<MtlTexture>(m_renderer, declaration, Mappings::TranslateExternalFormat(desiredFormat));
}

std::shared_ptr<IFrameBuffer> ResourceFactory::CreateFrameBuffer() const
{
    return std::make_shared<FrameBuffer>(m_renderer, m_renderer.GetRendererCapabilities().GetMaxNumberOfColorAttachments());
}

std::shared_ptr<IVertexArray> ResourceFactory::CreateVertexArray() const
{
    return std::make_shared<VertexArray>(m_renderer.GetRendererCapabilities());
}

std::shared_ptr<IBuffer> ResourceFactory::CreateBuffer(VertexBufferType type) const
{
    return std::make_shared<Buffer>(m_renderer, type);
}

std::shared_ptr<IBuffer> ResourceFactory::CreateBuffer(VertexBufferType type, std::span<const std::byte> data) const
{
    auto buffer = CreateBuffer(type);
    buffer->SetDataRaw(data);
    return buffer;
}

std::shared_ptr<IShaderProgram> ResourceFactory::CreateShaderProgramFromFiles(std::initializer_list<str> files) const
{
    class LibrariesRegistry
    {
    public:
        std::shared_ptr<ShaderLibrary> FindOrOpen(Renderer& renderer, std::filesystem::path path)
        {
            auto [it, inserted] = m_libraries.emplace(path, std::shared_ptr<ShaderLibrary>{});
            if (inserted)
                it->second = std::make_shared<ShaderLibrary>(renderer, LoadSource(path));
            
            return it->second;
        }
        
    private:
        std::string LoadSource(std::filesystem::path path)
        {
            std::ifstream in(path);
            in.exceptions(std::ifstream::failbit);
            return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        }
        
    private:
        std::map<std::filesystem::path, std::shared_ptr<ShaderLibrary>> m_libraries;
    };
    static LibrariesRegistry s_registry {};
    
    
    std::vector<std::filesystem::path> filenames;
    std::transform(files.begin(), files.end(), std::back_inserter(filenames), [](std::filesystem::path path){
        return path.extension() == ".glsl" ? path.replace_extension().replace_extension(".metal") : path;
    });
    
    std::sort(filenames.begin(), filenames.end());
    filenames.erase(std::unique(filenames.begin(), filenames.end()), filenames.end());
    
    for (const auto& path : filenames)
    {
        if (auto library = s_registry.FindOrOpen(m_renderer, path))
        {
            ShaderProgram::Descriptor descriptor {library, "vertex_main", "fragment_main"};
            
            return std::make_shared<ShaderProgram>(descriptor);
        }
    }
    
    return nullptr;
}

void AT2::Metal::ResourceFactory::ReloadResources(ReloadableGroup group)
{
	
}
