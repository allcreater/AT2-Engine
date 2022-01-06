#include "TextureLoader.h"

#include "../AT2.h"

#ifdef USE_GLI
#include <gli/gli.hpp>
#endif

#include <algorithm>
#include <fstream>
#include <filesystem>

#if !defined(USE_DEVIL) && defined(USE_PLATFORM_HACKS)
#include <cstdio>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace AT2;
using namespace AT2::Resources;

#ifndef USE_DEVIL
namespace
{
    std::tuple<ExternalTextureFormat, glm::ivec2> DetermineExternalFormat(std::span<const stbi_uc> buffer) 
    { 
        assert(buffer.size_bytes() < std::numeric_limits<int>::max());
        const auto intSize = static_cast<int>(buffer.size_bytes());

        ExternalTextureFormat format {TextureLayout::Red, BufferDataType::UByte};

        if (stbi_is_16_bit_from_memory(buffer.data(), intSize))
        {
            format.DataType = BufferDataType::UShort;
        }
        else if (stbi_is_hdr_from_memory(buffer.data(), intSize))
        {
            format.DataType = BufferDataType::Float;
        }

        int w = 0, h = 0, c = 0;
        stbi_info_from_memory(buffer.data(), intSize, &w, &h, &c);

        switch (c)
        {
            case 1: format.ChannelsLayout = TextureLayout::Red; break;
            case 2: format.ChannelsLayout = TextureLayout::RG; break;
            case 3: format.ChannelsLayout = TextureLayout::RGB; break;
            case 4: format.ChannelsLayout = TextureLayout::RGBA; break;
            default:
                throw std::logic_error("unsupported texture channel layout");
        };

        return {format, {w, h}};
    }
}; // namespace

TextureRef TextureLoader::LoadTexture(const std::shared_ptr<IRenderer>& renderer, const std::filesystem::path& path)
{
#ifdef USE_PLATFORM_HACKS
    // not so gracefully as with iostream, but much faster under debugger
#if defined(WIN32) || defined(_WIN32)
    if (std::unique_ptr<FILE, decltype(&fclose)> file {_wfopen(path.native().c_str(), L"rb"), fclose})
#else
    if (std::unique_ptr<FILE, decltype(&fclose)> file {std::fopen(reinterpret_cast<const char*>(path.c_str()), "rb"), fclose})
#endif
    {
        std::vector<std::byte> data { static_cast<size_t>(file_size(path))};
        fread(data.data(), 1, data.size(), file.get());

        return LoadTexture(renderer, data);
    }

    return nullptr;
#else
    std::basic_ifstream<char> stream {path, std::ios::binary};
    std::vector< char> data (static_cast<size_t>(file_size(path)));
    stream.read(data.data(), data.size());

    //std::vector<std::byte> data{std::istreambuf_iterator<std::byte>(file), {}};
    return LoadTexture(renderer, std::span{reinterpret_cast<const std::byte*>(data.data()), data.size()} );
#endif
}

TextureRef TextureLoader::LoadTexture(const std::shared_ptr<IRenderer>& renderer, std::span<const std::byte> rawData)
{
    auto data = Utils::reinterpret_span_cast<const stbi_uc>(rawData);
    auto [format, size] = DetermineExternalFormat(data);
    
    int requiredNumberOfChannels = 0; // default, no transformations
#ifdef __APPLE__
    if (format.ChannelsLayout == TextureLayout::RGB)
    {
        format.ChannelsLayout = TextureLayout::RGBA;
        requiredNumberOfChannels = 4;
    }
#endif
    
    auto parsedData = [&, format=format]() -> void* {
        int width, height;

        switch (format.DataType)
        {
        case BufferDataType::UByte: return stbi_load_from_memory(data.data(), data.size_bytes(), &width, &height, nullptr, requiredNumberOfChannels);
        case BufferDataType::UShort: return stbi_load_16_from_memory(data.data(), data.size_bytes(), &width, &height, nullptr, requiredNumberOfChannels);
        case BufferDataType::Float: return stbi_loadf_from_memory(data.data(), data.size_bytes(), &width, &height, nullptr, requiredNumberOfChannels);
        }

        return nullptr;
    }();

    TextureRef result = nullptr;
    if (parsedData)
    {
        const auto numMipmaps = static_cast<unsigned>(log(std::max(size.x, size.y)) / log(2));

        result = renderer->GetResourceFactory().CreateTexture(Texture2D {size, numMipmaps}, format);
        result->SubImage2D({0, 0}, glm::xy(size), 0, format, parsedData);
        result->BuildMipmaps();
        stbi_image_free(parsedData);
    }

    return result;
}
#endif
