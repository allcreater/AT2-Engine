#include "Application.h"

#include <Renderers/Metal/Renderer.h>

using namespace AT2;
#ifdef USE_SDL_INSTEADOF_SFML
	#include "SDL/application.h"
	using namespace AT2::SDL;
#else
	#include "GLFW/glfw_application.h"
	using namespace AT2::GLFW;
#endif


void AT2::SingleWindowApplication::Run(std::unique_ptr<WindowContextBase> windowContext)
{
    ContextParameters contextParams {
        .contextType = MetalContextParams{},
        .refresh_rate = 60
    };

    auto window = ConcreteApplication::get().createWindow(contextParams, {1280, 800});

    ConcreteApplication::get().OnNoActiveWindows = [] {
        ConcreteApplication::get().stop();
        //spdlog::info("Exit");
    };

    window->InitializeCallback = [window = window.get(), windowContext = windowContext.get()] {
        windowContext->m_window = window;
        windowContext->m_renderer = std::make_unique<Metal::Renderer>(window->getGraphicsContext().getPlatformSwapchain());
        windowContext->OnInitialized();
    };

    window->RenderCallback = [windowContext = windowContext.get()](Seconds dt) {
        windowContext->getRenderer()->BeginFrame();
        windowContext->OnRender(dt);
        windowContext->getRenderer()->FinishFrame();
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
