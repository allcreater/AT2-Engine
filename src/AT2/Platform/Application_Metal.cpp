#include "Application.h"

#include <Renderers/Metal/Renderer.h>
#include <GLFW/glfw_application.h>

using namespace AT2;

void GraphicsContext::Initialize(std::shared_ptr<IWindow> window) 
{
    m_window = std::move(window);

    m_renderer = std::make_unique<AT2::Metal::Renderer>();
    OnInitialized();
}

void AT2::SingleWindowApplication::Run(std::unique_ptr<GraphicsContext> graphicsContext)
{
    using namespace AT2::GLFW;
    auto glfwWindow = GlfwApplication::get().createWindow(std::nullopt, {1280, 800});

    GlfwApplication::get().OnNoActiveWindows = [] {
        GlfwApplication::get().stop();
        //spdlog::info("Exit");
    };

    glfwWindow->InitializeCallback = [glfwWindow, graphicsContext=graphicsContext.get()] {
        graphicsContext->Initialize(glfwWindow);
    };

    glfwWindow->RenderCallback = [graphicsContext = graphicsContext.get()](Seconds dt) {
        graphicsContext->OnRender(dt);
        graphicsContext->getRenderer()->FinishFrame();
    };

    glfwWindow->UpdateCallback = std::bind_front(&GraphicsContext::OnUpdate, graphicsContext.get());
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

    AT2::GLFW::GlfwApplication::get().run();
}