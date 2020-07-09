#include "InputHelper.h"
#include <algorithm>

using namespace AT2;
using namespace AT2::UI;
//
// UiInputHandler
//

void UiInputHandler::OnMouseMove(const MousePos& mousePos)
{
    m_mousePos = mousePos;

    bool eventCatched = false;

    m_rootNode->TraverseDepthFirst([&](const std::shared_ptr<Node>& node) {
        if (!eventCatched && isPointInsideNode(*node, m_mousePos.getPos()))
        {
            if (auto& vector = m_mouseDownOnControl[0];
                std::find_if(vector.begin(), vector.end(),
                             [&](const std::weak_ptr<Node>& n) { return n.lock() == node; }) != vector.end())
                if (EventClicked)
                    eventCatched |= EventMouseDrag(node, m_mousePos);
        }
    });
}

void UiInputHandler::OnMouseDown(int key)
{
    m_rootNode->TraverseDepthFirst([&](const std::shared_ptr<Node>& node) {
        if (isPointInsideNode(*node, m_mousePos.getPos()))
            m_mouseDownOnControl[key].push_back(node);
    });
}
void UiInputHandler::OnMouseUp(int key)
{
    bool eventCatched = false;

    if (key == 0)
    {
        m_rootNode->TraverseDepthFirst([&](const std::shared_ptr<Node>& node) {
            if (!eventCatched && isPointInsideNode(*node, m_mousePos.getPos()))
            {
                if (auto& vector = m_mouseDownOnControl[key];
                    std::find_if(vector.begin(), vector.end(),
                                 [&](const std::weak_ptr<Node>& n) { return n.lock() == node; }) != vector.end())
                    if (EventClicked)
                        eventCatched |= EventClicked(node);
            }
        });
    }

    m_mouseDownOnControl[key].clear();
}

void UiInputHandler::OnMouseScroll(const glm::vec2& scrollDir)
{
    bool eventCatched = false;

    m_rootNode->TraverseDepthFirst([&](const std::shared_ptr<Node>& node) {
        if (!eventCatched && isPointInsideNode(*node, m_mousePos.getPos()))
        {
            if (EventScrolled)
                eventCatched |= EventScrolled(node, m_mousePos, scrollDir);

            eventCatched = true;
        }
    });
}

bool UiInputHandler::isPointInsideNode(const Node& node, const glm::vec2& pos)
{
    return node.GetScreenPosition().IsPointInside(pos);
}
