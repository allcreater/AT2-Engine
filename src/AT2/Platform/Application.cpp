#include "Application.h"

#if defined(AT2_ACTIVE_RENDERER_METAL)
    #include <Renderers/Metal/Renderer.h>
    using Renderer = AT2::Metal::Renderer;
#elif defined(AT2_ACTIVE_RENDERER_OPENGL_4_5)
    #include <Renderers/OpenGL/GlRenderer.h>
    #define AT2_ACTIVE_RENDERER_OPENGL
    using Renderer = AT2::OpenGL::GlRenderer;
#else
    #include <Renderers/OpenGL_4.1/GlRenderer.h>
    #define AT2_ACTIVE_RENDERER_OPENGL
    using Renderer = AT2::OpenGL41::GlRenderer;
#endif

using namespace AT2;
#ifdef USE_SDL_INSTEADOF_GLFW
	#include "SDL/application.h"
	using namespace AT2::SDL;
#ifdef AT2_ACTIVE_RENDERER_OPENGL
	GLADloadproc openglFunctionsBinder = reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress);
#endif
#else
	#include "GLFW/glfw_application.h"

	using namespace AT2::GLFW;
#ifdef AT2_ACTIVE_RENDERER_OPENGL
	GLADloadproc openglFunctionsBinder = reinterpret_cast<GLADloadproc>(glfwGetProcAddress);
#endif
#endif


void AT2::SingleWindowApplication::Run(std::unique_ptr<WindowContextBase> windowContext)
{
    ContextParameters contextParams {
#if defined(AT2_ACTIVE_RENDERER_METAL)
        .contextType = MetalContextParams{},
#elif defined(AT2_ACTIVE_RENDERER_OPENGL_4_5)
    	.contextType = OpenGLContextParams{OpenglProfile::Core, 4, 5, true},
#else
    	.contextType = OpenGLContextParams{OpenglProfile::Core, 4, 1, true},
#endif
    	.refresh_rate = 60
    };

    auto window = ConcreteApplication::get().createWindow(contextParams, {1280, 800});

    ConcreteApplication::get().OnNoActiveWindows = [] {
        ConcreteApplication::get().stop();
        //spdlog::info("Exit");
    };

    window->InitializeCallback = [window = window.get(), windowContext = windowContext.get()] {
#if defined(AT2_ACTIVE_RENDERER_METAL)
        [[maybe_unused]] auto autoreleasePool = Metal::ConstructMetalObject<NS::AutoreleasePool>();
        auto renderer = std::make_unique<Metal::Renderer>(window->getGraphicsContext());
#else
        auto renderer = std::make_unique<Renderer>(window->getGraphicsContext(), openglFunctionsBinder);
#endif
        windowContext->m_window = window;
        windowContext->m_visualizationSystem = std::move(renderer);
        windowContext->OnInitialized(*windowContext->m_visualizationSystem);
    };

    window->RenderCallback = [windowContext = windowContext.get()](Seconds dt) {
#if defined(AT2_ACTIVE_RENDERER_METAL)
        [[maybe_unused]] auto autoreleasePool = Metal::ConstructMetalObject<NS::AutoreleasePool>();
#endif
        windowContext->m_visualizationSystem->BeginFrame();
        windowContext->OnRender(dt, *windowContext->m_visualizationSystem);
        windowContext->m_visualizationSystem->FinishFrame();
    };

    window->UpdateCallback = std::bind_front(&WindowContextBase::OnUpdate, windowContext.get());
    window->RefreshingCallback = std::bind_front(&WindowContextBase::OnWindowRefreshing, windowContext.get());
    window->KeyDownCallback = std::bind_front(&WindowContextBase::OnKeyDown, windowContext.get());
    window->KeyUpCallback = std::bind_front(&WindowContextBase::OnKeyUp, windowContext.get());
    window->KeyRepeatCallback = std::bind_front(&WindowContextBase::OnKeyRepeat, windowContext.get());
    window->ResizeCallback = std::bind_front(&WindowContextBase::OnResize, windowContext.get());
    window->ClosingCallback = std::bind_front(&WindowContextBase::OnClosing, windowContext.get());
    window->MouseMoveCallback = std::bind_front(&WindowContextBase::OnMouseMove, windowContext.get());
    window->MouseDownCallback = std::bind_front(&WindowContextBase::OnMouseDown, windowContext.get());
    window->MouseUpCallback = std::bind_front(&WindowContextBase::OnMouseUp, windowContext.get());
    window->MouseScrollCallback = std::bind_front(&WindowContextBase::OnMouseScroll, windowContext.get());

    window->setWindowContext(std::move(windowContext));

    ConcreteApplication::get().run();
}
