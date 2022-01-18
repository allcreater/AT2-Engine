#include <Platform/Application.h>

#include "UIHandler.h"

class UITest : public AT2::WindowContextBase
{
public:
    UITest() = default;

private:
    void OnInitialized( AT2::IVisualizationSystem& visualizationSystem ) override
    {
        getWindow().setLabel("Graph control demo").setCursorMode(CursorMode::Normal).setVSyncInterval(1);

        m_uiHub = std::make_unique<UiHub>();
        m_uiHub->Init(visualizationSystem);
        m_uiHub->Resize(getWindow().getSize());

        visualizationSystem.GetDefaultFramebuffer().SetClearColor(glm::vec4{});
        visualizationSystem.GetDefaultFramebuffer().SetClearDepth(0.0f);
    }

    void OnUpdate(const AT2::Seconds dt) override 
    {
    }

    void OnRender( const AT2::Seconds dt, AT2::IVisualizationSystem& visualizationSystem ) override
    {
        visualizationSystem.GetDefaultFramebuffer().Render([&](AT2::IRenderer& renderer) {
            renderer.GetStateManager().ApplyState(
                AT2::BlendMode {AT2::BlendFactor::SourceAlpha, AT2::BlendFactor::OneMinusSourceAlpha, glm::vec4 {1}});
            renderer.GetStateManager().ApplyState(AT2::FaceCullMode {});

            m_uiHub->Render(renderer, dt);
        });
    }

    void OnKeyPress(int key) {}

    void OnKeyDown (int key) override 
    {
        std::cout << "Key " << key << " down" << std::endl;

    	if (key == AT2::Keys::Key_Escape)
            getWindow().setCloseFlag(true);

        OnKeyPress(key);
    }

    void OnKeyRepeat(int key) override { OnKeyPress(key); }

    void OnResize (glm::ivec2 newSize) override
    {
        std::cout << "Size " << newSize.x << "x" << newSize.y << std::endl;
        if (m_uiHub)
            m_uiHub->Resize(newSize);
    };

    void OnMouseDown(int key) override { m_uiHub->GetInputHandler().OnMouseDown(key); }

    void OnMouseUp(int key) override 
    {
        std::cout << "Mouse " << key << std::endl;
        m_uiHub->GetInputHandler().OnMouseUp(key);
    }

    void OnMouseMove(const AT2::MousePos& pos) override { m_uiHub->GetInputHandler().OnMouseMove(pos); }

    void OnMouseScroll(const glm::dvec2& scrollDir) override 
    {
        std::cout << "Scroll " << scrollDir.y << std::endl;
        m_uiHub->GetInputHandler().OnMouseScroll(scrollDir);
    };

private:

    std::unique_ptr<UiHub> m_uiHub;
};

int main(int argc, char* argv[])
{
    try
    {
        AT2::SingleWindowApplication app;
        app.Run(std::make_unique<UITest>());
    }
    catch (AT2::AT2Exception& exception)
    {
        std::cout << "Runtime exception:" << exception.what() << std::endl;
        std::cin.get();
    }

    return 0;
}
