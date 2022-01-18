#include "graphics_context.h"

#include "utils.hpp"

using namespace AT2;
using namespace AT2::SDL;

namespace
{
    constexpr int TranslateOpenGlProfile(OpenglProfile profile)
    {
        switch (profile)
        {
        case OpenglProfile::Core: return SDL_GL_CONTEXT_PROFILE_CORE;
        case OpenglProfile::Compat: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
        default: throw std::domain_error("OpengProfile");
        }
    }
} // namespace

std::unique_ptr<IPlatformGraphicsContext> MakeOpenglContext(SDL_Window* window, const ContextParameters& contextParams)
{
    const auto& params = std::get<OpenGLContextParams>(contextParams.contextType);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, params.context_major_version);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, params.context_minor_version);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, contextParams.framebuffer_bits_red);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, contextParams.framebuffer_bits_green);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, contextParams.framebuffer_bits_blue);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, contextParams.framebuffer_bits_depth);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, contextParams.srgb_capable);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    const auto flags = !!params.debug_context * SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, TranslateOpenGlProfile(params.profile));

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, std::min(contextParams.msaa_samples, 1));

    SDL_GL_SetSwapInterval(contextParams.refresh_rate);

    class OpenglContext : public IPlatformGraphicsContext
    {
    public:
        OpenglContext(SDL_Window* window) : window {window}, context {SDL_GL_CreateContext(window)} {}
        ~OpenglContext() override { SDL_GL_DeleteContext(context); }

    private:
        SDL_Window* window;
        SDL_GLContext context;

        void setVSyncInterval(int interval) override { SDL_GL_SetSwapInterval(interval); }
        void* getPlatformSwapchain() const override { return nullptr; }
        glm::ivec2 getPhysicalViewportSize() const override
        {
            glm::ivec2 physicalSize;
            SDL_GL_GetDrawableSize(window, &physicalSize.x, &physicalSize.y);
            return physicalSize;
        }
        void makeCurrent() override { SDL_GL_MakeCurrent(window, context); }
        void swapBuffers() override { SDL_GL_SwapWindow(window); }
    };

    return std::make_unique<OpenglContext>(window);
}

//TODO
std::unique_ptr<IPlatformGraphicsContext> MakeMetalContext(SDL_Window* window, const ContextParameters& contextParams)
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_InitSubSystem(SDL_INIT_VIDEO); // TODO: do once?

    class MetalContext : public IPlatformGraphicsContext
    {
    public:
        MetalContext(SDL_Window* window) : window {window}, renderer {SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC)} {}
        ~MetalContext() override { SDL_DestroyRenderer(renderer); }

    private:
        SDL_Window* window;
        SDL_Renderer* renderer;

        void setVSyncInterval(int interval) override { }
        void* getPlatformSwapchain() const override { return SDL_RenderGetMetalLayer(renderer); }
        glm::ivec2 getPhysicalViewportSize() const override
        {
            glm::ivec2 physicalSize;
            SDL_Metal_GetDrawableSize(window, &physicalSize.x, &physicalSize.y);
            return physicalSize;
        }
        void makeCurrent() override {}
        void swapBuffers() override {}
    };

    return std::make_unique<MetalContext>(window);
}

std::unique_ptr<IPlatformGraphicsContext> MakeDummyContext(SDL_Window* window, const ContextParameters& contextParams)
{
    class DummyContext : public IPlatformGraphicsContext
    {
    public:
        DummyContext() = default;
        ~DummyContext() override = default;

    private:
        void setVSyncInterval(int interval) override {}
        void* getPlatformSwapchain() const override { return nullptr; }
        glm::ivec2 getPhysicalViewportSize() const override { return {}; }
        void makeCurrent() override {}
        void swapBuffers() override {}
    };

    return std::make_unique<DummyContext>();
}

std::unique_ptr<IPlatformGraphicsContext> SDL::MakeGraphicsContext(SDL_Window* window, const ContextParameters& contextParams)
{
    return std::visit(Utils::overloaded {
    	[=](const OpenGLContextParams&) { return MakeOpenglContext(window, contextParams); },
        [=](const MetalContextParams&) { return MakeMetalContext(window, contextParams); },
        [=](std::monostate) { return MakeDummyContext(window, contextParams); }
    }, contextParams.contextType);
}

Uint32 SDL::GetContextSpecificWindowFlags( const ContextParameters& contextParams )
{
    return std::visit( Utils::overloaded {
    	[](const OpenGLContextParams&) -> Uint32 { return SDL_WINDOW_OPENGL; },
        [](const MetalContextParams&) -> Uint32 { return 0; }, // SDL_WINDOW_METAL ?
        [](const auto&) -> Uint32 { return 0; }
    }, contextParams.contextType);
}
