#include <AT2/OpenGL/GlRenderer.h>
#include <AT2/OpenGL/GLFW/glfw_application.h>

#include "UIHandler.h"

class App
{
public:
    App()
    {
        GlfwApplication::get().OnNoActiveWindows = [] {
            GlfwApplication::get().stop();
            //spdlog::info("Exit");
        };


        m_window = GlfwApplication::get().createWindow();

        m_window->setLabel("Graph control demo").setSize({1024, 768}).setCursorMode(GlfwCursorMode::Normal);

        SetupWindowCallbacks();
    }

    void Run() { GlfwApplication::get().run(); }

private:
    void OnInitialize()
    {
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
            throw GlfwException("Failed to initialize GLEW"); //yes, it's strange to throw a Glfw exception :3

        m_renderer = std::make_unique<AT2::GlRenderer>();

        m_uiHub = std::make_unique<UiHub>();
        m_uiHub->Init(m_renderer);
        m_uiHub->Resize(m_window->getSize());

        //Init
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void OnRender(double dt)
    {
        m_renderer->SetViewport(AABB2d {{0, 0}, m_window->getSize()});
        m_renderer->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
        m_renderer->ClearDepth(0);

        m_uiHub->Render(m_renderer, dt);

        m_renderer->FinishFrame();
    }

    void OnKeyPress(int key) {}

    void SetupWindowCallbacks()
    {


        m_window->KeyDownCallback = [&](int key) {
            std::cout << "Key " << key << " down" << std::endl;

            switch (key)
            {
            case GLFW_KEY_R:
            {
                m_renderer->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
            }
            break;

            case GLFW_KEY_ESCAPE:
            {
                m_window->setCloseFlag(true);
            }
            break;
            }


            OnKeyPress(key);
        };

        m_window->KeyRepeatCallback = [&](int key) { OnKeyPress(key); };

        m_window->ResizeCallback = [&](const glm::ivec2& newSize) {
            std::cout << "Size " << newSize.x << "x" << newSize.y << std::endl;
            if (m_uiHub)
                m_uiHub->Resize(newSize);
        };

        m_window->MouseDownCallback = [&](int key) { m_uiHub->GetInputHandler().OnMouseDown(key); };

        m_window->MouseUpCallback = [&](int key) {
            std::cout << "Mouse " << key << std::endl;
            m_uiHub->GetInputHandler().OnMouseUp(key);
        };

        m_window->MouseMoveCallback = [&](const MousePos& pos) { m_uiHub->GetInputHandler().OnMouseMove(pos); };

        m_window->MouseScrollCallback = [&](const glm::vec2& scrollDir) {
            std::cout << "Scroll " << scrollDir.y << std::endl;
            m_uiHub->GetInputHandler().OnMouseScroll(scrollDir);
        };

        m_window->InitializeCallback = [&]() { m_window->setVSyncInterval(1); };

        m_window->ClosingCallback = [&]() { m_renderer->Shutdown(); };

        m_window->RenderCallback = std::bind_front(&App::OnRender, this);
        m_window->InitializeCallback = std::bind_front(&App::OnInitialize, this);
    }

private:
    std::shared_ptr<GlfwWindow> m_window;
    std::shared_ptr<AT2::IRenderer> m_renderer;

    std::unique_ptr<UiHub> m_uiHub;
};

int main(int argc, char* argv[])
{
    try
    {
        App app;
        app.Run();
    }
    catch (AT2::AT2Exception& exception)
    {
        std::cout << "Runtime exception:" << exception.what() << std::endl;
        system("PAUSE");
    }

    return 0;
}
