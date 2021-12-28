#include "window.h"
#include <cassert>
#include <mutex>

#include "application.h"
#include "utils.hpp"


using namespace AT2;
using namespace AT2::SDL;

namespace
{
	constexpr char windowDataKey_this[] = "AT2_this_window";

    constexpr int TranslateOpenGlProfile( AT2::OpenglProfile profile)
    {
	    switch ( profile )
	    {
        case AT2::OpenglProfile::Core: return SDL_GL_CONTEXT_PROFILE_CORE;
        case AT2::OpenglProfile::Compat: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
        default: 
            throw std::domain_error("OpengProfile");
	    }
    }


    std::unique_ptr<IPlatformGraphicContext> MakeOpenglContext(SDL_Window* window, const ContextParameters& contextParams)
    {
        const auto& params = std::get<OpenGLContextParams>(contextParams.contextType);

        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, params.context_major_version);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, params.context_minor_version);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, contextParams.framebuffer_bits_red);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, contextParams.framebuffer_bits_green);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, contextParams.framebuffer_bits_blue);
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, contextParams.srgb_capable);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

        const auto flags = !!params.debug_context * SDL_GL_CONTEXT_DEBUG_FLAG | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, TranslateOpenGlProfile(params.profile));

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, std::min(contextParams.msaa_samples, 1));

        SDL_GL_SetSwapInterval(contextParams.refresh_rate);

		class OpenglContext : public IPlatformGraphicContext
        {
		public:
            OpenglContext(SDL_Window* window) : window {window}, context {SDL_GL_CreateContext(window)} {}
            ~OpenglContext() override { SDL_GL_DeleteContext(context); }

		private:
            SDL_Window* window;
            SDL_GLContext context;

	        void makeCurrent() override { SDL_GL_MakeCurrent(window, context); }
            void swapBuffers() override { SDL_GL_SwapWindow(window); }

        };

        return std::make_unique<OpenglContext>(window);
    }

    //TODO
    std::unique_ptr<IPlatformGraphicContext> MakeMetalContext(SDL_Window* window, const ContextParameters& contextParams)
    {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
        SDL_InitSubSystem(SDL_INIT_VIDEO); // TODO: do once?

        class MetalContext : public IPlatformGraphicContext
        {
        public:
            MetalContext(SDL_Window* window) : window {window}, renderer {SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC)} {}
            ~MetalContext() override
            {
            	SDL_DestroyRenderer(renderer);
            }

        private:
            SDL_Window* window;
            SDL_Renderer* renderer;

	        void makeCurrent() override {}
	        void swapBuffers() override {}
        };

        return std::make_unique<MetalContext>(window);
    }

    std::unique_ptr<IPlatformGraphicContext> MakeDummyContext(SDL_Window* window, const ContextParameters& contextParams)
    {
        //class DummyContext : public IPlatformGraphicContext
        //{
        //public:
        //    DummyContext();
        //    ~DummyContext() override = default;

        //private:
        //    void makeCurrent() override {}
        //    void swapBuffers() override {}

        //};

        return nullptr; //std::make_unique<DummyContext>();
    }

	std::unique_ptr<IPlatformGraphicContext> MakeContext(SDL_Window* window , const ContextParameters& contextParams)
	{
	    return std::visit(
	        Utils::overloaded {
        		[=](const OpenGLContextParams& ) { return MakeOpenglContext(window, contextParams); },
				[=](std::monostate) {return MakeDummyContext( window, contextParams );}
	        }, contextParams.contextType);
	}

}




Window* Window::FromNativeWindow(SDL_Window* window)
{
    auto* const frontendPtr = static_cast<Window*>(SDL_GetWindowData(window, windowDataKey_this));
    assert(frontendPtr);
    return frontendPtr;
}

Window::Window(const ContextParameters& contextParameters, glm::ivec2 initialSize)
	: WindowBase{ initialSize}
{
    const Uint32 additionalFlags =
        std::visit(Utils::overloaded {
        	[](const OpenGLContextParams&) -> Uint32{ return SDL_WINDOW_OPENGL; }, 
            [](const auto&) -> Uint32{ return 0; }
    }, contextParameters.contextType);

    window_impl = SDL_CreateWindow(window_label.c_str(), -1, -1, window_size.x, window_size.y, SDL_WINDOW_ALLOW_HIGHDPI | additionalFlags);
    if (!window_impl)
        throw Exception("Window creation failed");

    graphicsContext = MakeContext(window_impl, contextParameters);

    SDL_SetWindowData(window_impl, windowDataKey_this, this);
}

bool Window::isKeyDown(int keyCode) const
{
    return window_impl && SDL_GetKeyboardState(nullptr)[keyCode];
}

bool Window::isMouseKeyDown(int button) const
{
    return false;
    //return window_impl && (glfwGetMouseButton(window_impl, button) == GLFW_PRESS);
}

Window& Window::setCursorMode(CursorMode cursorMode)
{
    ConcreteApplication::get().postAction([=] {
        if (!window_impl)
            return;

        SDL_ShowCursor(cursorMode == CursorMode::Normal ? SDL_ENABLE : SDL_DISABLE);

        auto relativeMode = static_cast<SDL_bool>(cursorMode == CursorMode::Disabled);
        
        //SDL_SetWindowGrab(window_impl, relativeMode);
        SDL_SetRelativeMouseMode(relativeMode);
    });

    return *this;
}

void Window::UpdateAndRender()
{
    assert(window_impl);

    graphicsContext->makeCurrent();

    {
        if (!is_initialized)
        {
            is_initialized = true;
            OnInitialize();
            OnResize(getSize());
        }

        
        const auto currentTime = Seconds {SDL_GetTicks64() / 1000.0 };
        OnUpdate(currentTime - previous_render_time);
        if (getSize().x > 0 && getSize().y > 0)
            OnRender(currentTime - previous_render_time);

        previous_render_time = currentTime;
    }

    graphicsContext->swapBuffers();
}

Window& Window::setLabel(std::string label)
{
    {
        std::lock_guard lock {mutex};
        window_label = std::move(label);
    }

    ConcreteApplication::get().postAction([this] {
        if (window_impl != nullptr)
            SDL_SetWindowTitle(window_impl, window_label.c_str());
    });

    return *this;
}

Window& Window::setVSyncInterval(int interval)
{
    std::lock_guard lock {mutex};

    swap_interval = interval;
    swap_interval_need_update = true;

    return *this;
}

void Window::requestAttention()
{
    ConcreteApplication::get().postAction([=] {
        if (window_impl)
            SDL_RaiseWindow(window_impl);
    });
}

Window& Window::setSize(glm::ivec2 size)
{
    {
        std::lock_guard lock {mutex};
        window_size = size;
    }

    ConcreteApplication::get().postAction([=] {
        if (window_impl != nullptr)
            SDL_SetWindowSize(window_impl, size.x, size.y);
    });

    return *this;
}

void Window::Close()
{
    std::lock_guard lock {mutex};

    assert(window_impl);

    OnClosing();
    SDL_DestroyWindow(window_impl);
    window_impl = nullptr;
}
