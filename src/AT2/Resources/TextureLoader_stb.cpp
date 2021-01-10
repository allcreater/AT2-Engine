#include "TextureLoader.h"

#include "../AT2.h"

#ifdef USE_GLI
#include <gli/gli.hpp>
#endif

#include <algorithm>
#include <fstream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace AT2;
using namespace AT2::Resources;

#ifndef USE_DEVIL
namespace
{
    std::tuple<ExternalTextureFormat, glm::ivec2> DetermineExternalFormat(std::span<const stbi_uc> buffer) 
    { 
        ExternalTextureFormat format {TextureLayout::Red, BufferDataType::UByte};

        if(stbi_is_16_bit_from_memory(buffer.data(), buffer.size_bytes()))
        {
            format.DataType = BufferDataType::UShort;
        }
        else if (stbi_is_hdr_from_memory(buffer.data(), buffer.size_bytes()))
        {
            format.DataType = BufferDataType::Float;
        }

        int w = 0, h = 0, c = 0;
        stbi_info_from_memory(buffer.data(), buffer.size_bytes(), &w, &h, &c);

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
    std::basic_ifstream<std::byte> stream {path, std::ios::binary};
    std::vector<std::byte> data {static_cast<size_t>(file_size(path))};
    stream.read(data.data(), data.size());

    //std::vector<std::byte> data{std::istreambuf_iterator<std::byte>(file), {}};
    return LoadTexture(renderer, data);
}

TextureRef TextureLoader::LoadTexture(const std::shared_ptr<IRenderer>& renderer, std::span<const std::byte> rawData)
{
    auto data = Utils::reinterpret_span_cast<const stbi_uc>(rawData);
    auto [format, size] = DetermineExternalFormat(data);

    auto parsedData = [&]() -> void* {
        int width, height;

        switch (format.DataType)
        {
        case BufferDataType::UByte: return stbi_load_from_memory(data.data(), data.size_bytes(), &width, &height, nullptr, 0);
        case BufferDataType::UShort: return stbi_load_16_from_memory(data.data(), data.size_bytes(), &width, &height, nullptr, 0);
        case BufferDataType::Float: return stbi_loadf_from_memory(data.data(), data.size_bytes(), &width, &height, nullptr, 0);
        }

        return nullptr;
    }();

    TextureRef result = nullptr;
    if (parsedData)
    {
        const unsigned numMipmaps = log(std::max({size.x, size.y})) / log(2);

        result = renderer->GetResourceFactory().CreateTexture(Texture2D {size, numMipmaps}, format);
        result->SubImage2D({0, 0}, glm::xy(size), 0, format, parsedData);
        result->BuildMipmaps();
        stbi_image_free(parsedData);
    }

    return result;
}
#endif