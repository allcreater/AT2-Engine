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

void GraphicsContext::Initialize(std::shared_ptr<IWindow> window) 
{
    m_window = std::move(window);

    m_renderer = std::make_unique<AT2::Metal::Renderer>(m_window->getNativeSwapchain());
    OnInitialized();
}

void AT2::SingleWindowApplication::Run(std::unique_ptr<GraphicsContext> windowContext)
{
    auto window = ConcreteApplication::get().createWindow(ContextParameters{MetalContextParams{}}, {1280, 800});

    ConcreteApplication::get().OnNoActiveWindows = [] {
        ConcreteApplication::get().stop();
        //spdlog::info("Exit");
    };

    window->InitializeCallback = [window, windowContext=windowContext.get()] {
        windowContext->Initialize(window);
    };

    window->RenderCallback = [windowContext = windowContext.get()](Seconds dt) {
        windowContext->OnRender(dt);
        windowContext->getRenderer()->FinishFrame();
    };

    window->UpdateCallback = std::bind_front(&GraphicsContext::OnUpdate, windowContext.get());
    window->RefreshingCallback = std::bind_front(&GraphicsContext::OnWindowRefreshing, windowContext.get());
    window->KeyDownCallback = std::bind_front(&GraphicsContext::OnKeyDown, windowContext.get());
    window->KeyUpCallback = std::bind_front(&GraphicsContext::OnKeyUp, windowContext.get());
    window->KeyRepeatCallback = std::bind_front(&GraphicsContext::OnKeyRepeat, windowContext.get());
    window->ResizeCallback = std::bind_front(&GraphicsContext::OnResize, windowContext.get());
    window->ClosingCallback = std::bind_front(&GraphicsContext::OnClosing, windowContext.get());
    window->MouseMoveCallback = std::bind_front(&GraphicsContext::OnMouseMove, windowContext.get());
    window->MouseDownCallback = std::bind_front(&GraphicsContext::OnMouseDown, windowContext.get());
    window->MouseUpCallback = std::bind_front(&GraphicsContext::OnMouseUp, windowContext.get());
    window->MouseScrollCallback = std::bind_front(&GraphicsContext::OnMouseScroll, windowContext.get());

    window->setWindowContext(std::move(windowContext));

    ConcreteApplication::get().run();
}
