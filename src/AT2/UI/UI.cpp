#include "UI.h"

#include <algorithm>

using namespace AT2::UI;

//
// Node
//

void Node::Measure(const glm::ivec2 & position, const glm::uvec2 & possibleSize)
{
	m_CanvasData.MeasuredSize = possibleSize;
	m_CanvasData.Position = position;
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

std::shared_ptr<StackPanel> StackPanel::Make(std::string_view name, Alignment alignment, std::initializer_list<std::shared_ptr<Node>> children, const glm::ivec2& size)
{
	auto ptr = std::shared_ptr<StackPanel>(new StackPanel(name, alignment, size));
	ptr->Initialize(children);
	return ptr;
}

std::shared_ptr<Plot> Plot::Make(std::string_view name, const glm::ivec2& size)
{
	return std::shared_ptr<Plot>(new Plot(name, size));
}

std::shared_ptr<Button> Button::Make(std::string_view name, const glm::ivec2& size)
{
	return std::shared_ptr<Button>(new Button(name, size));
}