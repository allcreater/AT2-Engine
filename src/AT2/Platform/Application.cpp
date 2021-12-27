#include "Application.h"

#include <Renderers/OpenGL/GlRenderer.h>

#ifdef USE_SDL_INSTEADOF_SFML
	#include <SDL/application.h>
	using namespace AT2::SDL;

	GLADloadproc openglFunctionsBinder = nullptr;
#else
	#include <GLFW/glfw_application.h>

	using namespace AT2::GLFW;
	GLADloadproc openglFunctionsBinder = reinterpret_cast<GLADloadproc>(glfwGetProcAddress);
#endif

using namespace AT2;

void GraphicsContext::Initialize(std::shared_ptr<IWindow> window) 
{
    m_window = std::move(window);

    m_renderer = std::make_unique<OpenGL::GlRenderer>(openglFunctionsBinder);
    OnInitialized();
}

void AT2::SingleWindowApplication::Run(std::unique_ptr<GraphicsContext> graphicsContext)
{
    ContextParameters contextParams {
    	.contextType = OpenGLContextParams{OpenglProfile::Core, 4, 5, true},
    	.refresh_rate = 60
    };

    auto glfwWindow = ConcreteApplication::get().createWindow(contextParams, {1280, 800});

    ConcreteApplication::get().OnNoActiveWindows = [] {
        ConcreteApplication::get().stop();
        //spdlog::info("Exit");
    };

    glfwWindow->InitializeCallback = [glfwWindow, graphicsContext=graphicsContext.get()] {
        graphicsContext->Initialize(glfwWindow);
    };

    glfwWindow->UpdateCallback = std::bind_front(&GraphicsContext::OnUpdate, graphicsContext.get());
    glfwWindow->RenderCallback = std::bind_front(&GraphicsContext::OnRender, graphicsContext.get());
    glfwWindow->RefreshingCallback = std::bind_front(&GraphicsContext::OnWindowRefreshing, graphicsContext.get());
    glfwWindow->KeyDownCallback = std::bind_front(&GraphicsContext::OnKeyDown, graphicsContext.get());
    glfwWindow->KeyUpCallback = std::bind_front(&GraphicsContext::OnKeyUp, graphicsContext.get());
    glfwWindow->KeyRepeatCallback = std::bind_front(&GraphicsContext::OnKeyRepeat, graphicsContext.get());
    glfwWindow->ResizeCallback = std::bind_front(&GraphicsContext::OnResize, graphicsContext.get());
    glfwWindow->ClosingCallback = std::bind_front(&GraphicsContext::OnClosing, graphicsContext.get());
    glfwWindow->MouseMoveCallback = std::bind_front(&GraphicsContext::OnMouseMove, graphicsContext.get());
    glfwWindow->MouseDownCallback = std::bind_front(&GraphicsContext::OnMouseDown, graphicsContext.get());
    glfwWindow->MouseUpCallback = std::bind_front(&GraphicsContext::OnMouseUp, graphicsContext.get());
    glfwWindow->MouseScrollCallback = std::bind_front(&GraphicsContext::OnMouseScroll, graphicsContext.get());

    glfwWindow->setWindowContext(std::move(graphicsContext));

    ConcreteApplication::get().run();
}
