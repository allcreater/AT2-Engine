#ifndef UI_INPUT_VISITOR_HEADER
#define UI_INPUT_VISITOR_HEADER

#include "UI.h"

namespace AT2::UI
{

//	class MouseClickVisitor : public UiVisitor
//	{
//	public:
//		MouseClickVisitor(const glm::vec2& mousePos) : m_mousePos(mousePos)
//		{
//		}
//
//		void Visit(Node& node) override
//		{
//			UiVisitor::Visit(node);
//		}
//		void Visit(Group& node) override
//		{
//			UiVisitor::Visit(node);
//		}
//		void Visit(StackPanel& node) override
//		{
//			UiVisitor::Visit(node);
//		}
//		void Visit(Button& node) override
//		{
//			UiVisitor::Visit(node);
//			Work(node);
//		}
//		void Visit(Plot& node) override
//		{
//			//
//			//glViewport(node.GetCanvasData().Position.x, node.GetCanvasData().Position.y, node.GetCanvasData().MeasuredSize.x, node.GetCanvasData().MeasuredSize.y);
//
//			UiVisitor::Visit(node);
//			Work(node);
//		}
//
//		void Work(Node& node)
//		{
//			auto aabb = AABB2d(node.GetCanvasData().Position, node.GetCanvasData().Position + glm::ivec2(node.GetCanvasData().MeasuredSize));
//			if (!m_eventCatched && aabb.IsPointInside(m_mousePos))
//			{
//				if (node.EventClicked)
//					node.EventClicked(node);
//
//				m_eventCatched = true;
//			}
//		}
//
//	private:
//		bool m_eventCatched = false;
//		glm::vec2 m_mousePos;
//	};
//
}

#endif