#pragma once

#include "../Foundation.h"

#include <SDL2/SDL.h>

namespace AT2::SDL
{
    std::unique_ptr<IPlatformGraphicsContext> MakeGraphicsContext(SDL_Window* window, const ContextParameters& contextParams);
    Uint32 GetContextSpecificWindowFlags(const ContextParameters& contextParams);

} // namespace AT2::SDL
