#include "TextureLoader.h"

#include "../AT2.h"

#ifdef USE_GLI
#include <gli/gli.hpp>
#endif

#include <algorithm>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace AT2;
using namespace AT2::Resources;

namespace
{
    static ExternalTextureFormat DetermiteExternalFormat(const stbi_uc* buffer, int len) 
    { 
        ExternalTextureFormat format;
        format.DataType = BufferDataType::UByte;

        if(stbi_is_16_bit_from_memory(buffer, len))
        {
            format.DataType = BufferDataType::UShort;
        }
        else if (stbi_is_hdr_from_memory(buffer, len))
        {
            format.DataType = BufferDataType::Float;
        }

        int w, h, c;
        stbi_info_from_memory(buffer, len, &w, &h, &c);

        switch (c)
        {
            case 1: format.ChannelsLayout = TextureLayout::Red; break;
            case 2: format.ChannelsLayout = TextureLayout::RG; break;
            case 3: format.ChannelsLayout = TextureLayout::RGB; break;
            case 4: format.ChannelsLayout = TextureLayout::RGBA; break;
            default: 0;//TODO: the heck am I reading
        };

        return format;
    }
}; // namespace

TextureRef TextureLoader::LoadTexture(const std::shared_ptr<IRenderer>& renderer, const str& filename)
{
    std::basic_ifstream<std::byte> file {filename, std::ios::binary};
    std::vector<std::byte> data{std::istreambuf_iterator<std::byte>(file), {}};

    return LoadTexture(renderer, data);
}

TextureRef TextureLoader::LoadTexture(const std::shared_ptr<IRenderer>& renderer, std::span<const std::byte> data)
{
    auto format = DetermiteExternalFormat(reinterpret_cast<const stbi_uc*>(&data[0]), static_cast<int>(data.size()));

    void* parsed_data = nullptr;
    int width, height;

    if (format.DataType == BufferDataType::UByte)
    {
        parsed_data = reinterpret_cast<void*>(
            stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(&data[0]), 
                static_cast<int>(data.size()), &width, &height, 0, 0));
    }
    else if (format.DataType == BufferDataType::UShort)
    {
        parsed_data = reinterpret_cast<void*>(stbi_load_16_from_memory(
            reinterpret_cast<const stbi_uc*>(&data[0]), static_cast<int>(data.size()), &width, &height, 0, 0));
    }
    else if (format.DataType == BufferDataType::Float)
    {
        parsed_data = reinterpret_cast<void*>(stbi_loadf_from_memory(
            reinterpret_cast<const stbi_uc*>(&data[0]), static_cast<int>(data.size()), &width, &height, 0, 0));
    }

    TextureRef result = nullptr;

    if (parsed_data)
    {
        const auto size = glm::uvec3(width, height, 0);
        result = renderer->GetResourceFactory().CreateTexture(Texture2D {glm::xy(size), 1}, format);
        stbi_image_free(parsed_data);
    }

    return result;
}
