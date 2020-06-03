#pragma once

#include "AT2.h"

namespace AT2
{

class TextureLoader
{
public:
    //TODO: make async
    static std::shared_ptr<ITexture> LoadTexture(IRenderer&, const str& filename);
    static std::shared_ptr<ITexture> LoadTexture(IRenderer&, void* data, size_t size);
};

}