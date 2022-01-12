#pragma once

#include <filesystem>
#include <AT2.h>

namespace AT2::Resources
{
    class TextureLoader
    {
    public:
        //TODO: make async
        //TODO: caching!!!
        static TextureRef LoadTexture(IVisualizationSystem& renderer, const std::filesystem::path& path);
        static TextureRef LoadTexture(IVisualizationSystem& renderer, std::span<const std::byte> data);
    };

} // namespace AT2