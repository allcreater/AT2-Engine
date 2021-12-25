#ifndef UI_HANDLER_HEADER
#define UI_HANDLER_HEADER

#include "UI.h"

#include <list>

#include <Mesh.h>

//TODO: move into library
namespace AT2::UI
{
    class UiRenderer
    {
    public:
        UiRenderer(const std::shared_ptr<AT2::IRenderer>& renderer, std::shared_ptr<AT2::UI::Node> node);
        void Draw();
        void SetWindowSize(const glm::uvec2& windowSize) { m_windowSize = windowSize; }

    private:
        void RenderNode(const Node& node);
        glm::vec4 DebugColor(const Node& node);

    private:
        std::shared_ptr<AT2::Mesh> m_quadDrawable;
        std::weak_ptr<AT2::IRenderer> m_renderer;
        std::shared_ptr<AT2::UI::Node> m_uiRoot;
        glm::uvec2 m_windowSize;
    };

} // namespace AT2::UI


struct IAnimation
{
    virtual void Animate(AT2::Seconds dt) = 0;
    virtual bool IsFinished() = 0;

    virtual ~IAnimation() = default;
};

struct UiHub
{
    UiHub() = default;

    AT2::UI::UiInputHandler& GetInputHandler() { return *m_uiInputHandler; }

    void Init(const std::shared_ptr<AT2::IRenderer>& renderer);
    void Render(const std::shared_ptr<AT2::IRenderer>& renderer, AT2::Seconds dt);
    void Resize(const glm::ivec2& newSize);

private:
    const std::string DataSet1 = "DataSet #1";
    const std::string DataSet2 = "DataSet #2";

private:
    glm::ivec2 m_windowSize = {};

    std::shared_ptr<AT2::UI::Node> m_uiRoot;
    std::shared_ptr<AT2::UI::Plot> m_plotNode;

    std::unique_ptr<AT2::UI::UiRenderer> m_uiRenderer;
    std::unique_ptr<AT2::UI::UiInputHandler> m_uiInputHandler;

    std::list<std::unique_ptr<IAnimation>> m_animationsList;
};

#endif
