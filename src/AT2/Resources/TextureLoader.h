#pragma once

#include "../AT2.h"

namespace AT2::Resources
{
    class TextureLoader
    {
    public:
        //TODO: make async
        //TODO: caching!!!
        static TextureRef LoadTexture(const std::shared_ptr<IRenderer>& renderer, const str& filename);
        static TextureRef LoadTexture(const std::shared_ptr<IRenderer>& renderer, std::span<const std::byte> data);
    };

} // namespace AT2