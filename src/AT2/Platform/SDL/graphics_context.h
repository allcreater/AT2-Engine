#pragma once

#include "../Foundation.h"

#include <SDL2/SDL.h>

namespace AT2::SDL
{
    struct IPlatformGraphicsContext
    {
        virtual ~IPlatformGraphicsContext() = default;

        virtual void makeCurrent() = 0;
        virtual void swapBuffers() = 0;
    };

    std::unique_ptr<IPlatformGraphicsContext> MakeGraphicsContext(SDL_Window* window, const ContextParameters& contextParams);
    Uint32 GetContextSpecificWindowFlags(const ContextParameters& contextParams);

} // namespace AT2::SDL