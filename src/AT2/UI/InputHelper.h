#ifndef UI_INPUT_VISITOR_HEADER
#define UI_INPUT_VISITOR_HEADER

#include <utility>


#include "../OpenGL/GLFW/callback_types.h" //TODO: detach type MousePos from GLFW
#include "UI.h"

namespace AT2::UI
{

    //determines some UI events and rise callbacks
    class UiInputHandler
    {
    public:
        UiInputHandler(std::shared_ptr<AT2::UI::Node> rootNode) : m_rootNode(std::move(rootNode)) {}

    public:
        std::function<bool(const std::shared_ptr<Node>& node)> EventClicked;
        std::function<bool(const std::shared_ptr<Node>& node, const MousePos& mousePos)> EventMouseDrag;
        std::function<bool(const std::shared_ptr<Node>& node, const MousePos& mousePos, const glm::vec2& scrollDir)>
            EventScrolled;

        void OnMouseMove(const MousePos& mousePos);
        void OnMouseDown(int key);
        void OnMouseUp(int key);

        void OnMouseScroll(const glm::vec2& scrollDir);

    protected:
        bool isPointInsideNode(const Node& node, const glm::vec2& pos);

    private:
        std::shared_ptr<Node> m_rootNode;
        MousePos m_mousePos {};
        std::map<int, std::vector<std::weak_ptr<Node>>>
            m_mouseDownOnControl; //yes, it's an overkill, but it's a simplest solution
    };

} // namespace AT2::UI

#endif
