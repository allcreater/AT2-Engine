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
    using DescriptorDataPair = std::pair<Texture2D, void*>;
    DescriptorDataPair LoadFromBuffer(std::span<const stbi_uc> buffer) 
    { 
        constexpr static std::array layouts_8 {TextureFormat::R8Unorm, TextureFormat::RG8Unorm, TextureFormat::RGBA8Unorm, TextureFormat::RGBA8Unorm};
        constexpr static std::array layouts_16 {TextureFormat::R16Unorm, TextureFormat::RG16Unorm, TextureFormat::RGBA16Unorm, TextureFormat::RGBA16Unorm};
        constexpr static std::array layouts_32f {TextureFormat::R32Float, TextureFormat::RG32Float, TextureFormat::RGBA32Float, TextureFormat::RGBA32Float};

        assert(buffer.size_bytes() < std::numeric_limits<int>::max());
        const auto intSize = static_cast<int>(buffer.size_bytes());

        int width = 0, height = 0, numOfChannels = 0;
        stbi_info_from_memory(buffer.data(), intSize, &width, &height, &numOfChannels);

        assert(numOfChannels > 0 && numOfChannels <= 4);

        return [&]{
            const auto doRead = [&](const auto& loaderFunc, const auto& layoutLookup)
            {
                int readed_width = 0, readed_height = 0;
                void* readedData = loaderFunc(buffer.data(), buffer.size_bytes(), &readed_width, &readed_height, nullptr, numOfChannels == 3 ? 4 : 0);

                assert(readed_height == height && readed_width == width);

                const auto numMipmaps = static_cast<unsigned>(log(std::max(readed_height, readed_width)) / log(2));
                return DescriptorDataPair{Texture2D{ layoutLookup[numOfChannels-1], {readed_width, readed_height}, numMipmaps}, readedData};
            };

            if (stbi_is_16_bit_from_memory(buffer.data(), intSize))
                return doRead(stbi_load_16_from_memory, layouts_16);
            else if (stbi_is_hdr_from_memory(buffer.data(), intSize))
                return doRead(stbi_loadf_from_memory, layouts_32f);
            else 
                return doRead(stbi_load_from_memory, layouts_8);
        }();
    }
} // namespace

TextureRef TextureLoader::LoadTexture(IVisualizationSystem& renderer, const std::filesystem::path& path)
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
    std::basic_ifstream<char> stream {path, std::ios::binary | std::ios::in | std::ios::ate};
    const auto dataLength = [&stream](){
        auto pos = stream.tellg();
        stream.seekg(std::ios::beg);
        return static_cast<size_t>(pos);
    }();

    std::vector<char> data(dataLength);
    data.assign(std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{});

    return LoadTexture(renderer, std::as_bytes(std::span{data}));
#endif
}

TextureRef TextureLoader::LoadTexture(IVisualizationSystem& renderer, std::span<const std::byte> rawData)
{
    auto [texDesc, data] = LoadFromBuffer(Utils::reinterpret_span_cast<const stbi_uc>(rawData));

    TextureRef result = nullptr;
    if (data)
    {
        result = renderer.GetResourceFactory().CreateTexture(texDesc, false);
        result->SubImage2D({0, 0}, texDesc.getSize(), 0, texDesc.getFormat(), data);
        result->BuildMipmaps();
        stbi_image_free(data);
    }

    return result;
}
#endif
