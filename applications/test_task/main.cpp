#include <glad/glad.h>
#include <Platform/Application.h>

#include "UIHandler.h"

class UITest : public AT2::GraphicsContext
{
public:
    UITest() = default;

private:
    void OnInitialized() override
    {
        getWindow().setLabel("Graph control demo").setCursorMode(CursorMode::Normal);

        m_uiHub = std::make_unique<UiHub>();
        m_uiHub->Init(getRenderer());
        m_uiHub->Resize(getWindow().getSize());


        getWindow().setVSyncInterval(1);
        //Init
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void OnUpdate(const AT2::Seconds dt) override 
    {
    }

    void OnRender(const AT2::Seconds dt) override
    {
        getRenderer()->SetViewport(AABB2d {{0, 0}, getWindow().getSize()});
        getRenderer()->ClearBuffer(glm::vec4(0.0, 0.0, 0.0, 0.0));
        getRenderer()->ClearDepth(0);

        m_uiHub->Render(getRenderer(), dt);

        getRenderer()->FinishFrame();
    }

    void OnKeyPress(int key) {}

    void OnKeyDown (int key) override 
    {
        std::cout << "Key " << key << " down" << std::endl;

    	if (key == AT2::Keys::Key_R)
            getRenderer()->GetResourceFactory().ReloadResources(AT2::ReloadableGroup::Shaders);
        else if (key == AT2::Keys::Key_Escape)
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
        system("PAUSE");
    }

    return 0;
}
