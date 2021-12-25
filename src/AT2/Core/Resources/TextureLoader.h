#pragma once

#include <filesystem>
#include <AT2/AT2.h>

namespace AT2::Resources
{
    class TextureLoader
    {
    public:
        //TODO: make async
        //TODO: caching!!!
        static TextureRef LoadTexture(const std::shared_ptr<IRenderer>& renderer, const std::filesystem::path& path);
        static TextureRef LoadTexture(const std::shared_ptr<IRenderer>& renderer, std::span<const std::byte> data);
    };

} // namespace AT2