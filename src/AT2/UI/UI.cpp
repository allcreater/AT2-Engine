#include "UI.h"

#include <algorithm>

using namespace AT2::UI;

//
// Node
//

void Node::Measure(const glm::ivec2 & position, const glm::uvec2 & possibleSize)
{
	m_CanvasData.MeasuredSize.x = (HorizontalAlignment == Alignment::Stretch) ? possibleSize.x : std::min(possibleSize.x, m_Size.x);
	m_CanvasData.MeasuredSize.y = (VerticalAlignment == Alignment::Stretch) ? possibleSize.t : std::min(possibleSize.y, m_Size.y);
	
	switch (HorizontalAlignment)
	{
		case Alignment::Stretch:
			[[fallthrough]];
		case Alignment::Side1:
			m_CanvasData.Position.x = position.x; break;
		case Alignment::Side2:
			m_CanvasData.Position.x = position.x + (possibleSize.x - m_CanvasData.MeasuredSize.x); break;
		case Alignment::Center:
			m_CanvasData.Position.x = position.x + (possibleSize.x - m_CanvasData.MeasuredSize.x) / 2; break;
		default:
			throw AT2::AT2Exception(AT2Exception::ErrorCase::UI, "Incorrect HorizontalAlignment");
	}

	switch (VerticalAlignment)
	{
		case Alignment::Stretch:
			[[fallthrough]];
		case Alignment::Side1:
			m_CanvasData.Position.y = position.y; break;
		case Alignment::Side2:
			m_CanvasData.Position.y = position.y + (possibleSize.y - m_CanvasData.MeasuredSize.y); break;
		case Alignment::Center:
			m_CanvasData.Position.y = position.y + (possibleSize.y - m_CanvasData.MeasuredSize.y) / 2; break;
		default:
			throw AT2::AT2Exception(AT2Exception::ErrorCase::UI, "Incorrect VerticalAlignment");
	}
}

//
// Group
//

void Group::Initialize(std::initializer_list<std::shared_ptr<Node>> children)
{
	assert(m_Children.empty());
	m_Children = decltype(m_Children)(children.begin(), children.end());

	for (auto child : children)
	{
		assert(child->m_Parent.expired() == true);
		child->m_Parent = weak_from_this();
	}
}

void Group::AddChild(std::shared_ptr<Node> newChild)
{
	assert(std::find(m_Children.begin(), m_Children.end(), newChild) == m_Children.end());

	if (auto oldParent = newChild->m_Parent.lock())
		oldParent->RemoveChild(newChild);

	newChild->m_Parent = weak_from_this();
	m_Children.push_back(newChild);
}

bool Group::RemoveChild(const std::shared_ptr<Node>& child)
{
	auto iterator = std::remove(m_Children.begin(), m_Children.end(), child);
	auto numDeleted = std::distance(iterator, m_Children.end());
	m_Children.erase(iterator, m_Children.end());

	assert(numDeleted < 2); //means that there was duplicates but it don't supported
	return numDeleted > 0;
}

void Group::ForEachChild(std::function<void(std::shared_ptr<Node>&)> func)
{
	for (auto child : m_Children)
		func(child);
}

//
// Constructor methods
//

std::shared_ptr<StackPanel> StackPanel::Make(std::string_view name, Orientation orientation, std::initializer_list<std::shared_ptr<Node>> children, const glm::uvec2& size)
{
	auto ptr = std::shared_ptr<StackPanel>(new StackPanel(name, orientation, size));
	ptr->Initialize(children);
	return ptr;
}

std::shared_ptr<Plot> Plot::Make(std::string_view name, const glm::uvec2& size)
{
	return std::shared_ptr<Plot>(new Plot(name, size));
}

std::shared_ptr<Button> Button::Make(std::string_view name, const glm::uvec2& size, Alignment vertical, Alignment horizontal)
{
	auto ptr = std::shared_ptr<Button>(new Button(name, size));
	ptr->VerticalAlignment = vertical;
	ptr->HorizontalAlignment = horizontal;
	return ptr;
}