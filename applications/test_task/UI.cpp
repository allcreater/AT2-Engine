#include "UI.h"

//TODO: write unit tests

UI::Group::Group(std::initializer_list<Node*> children)
	: m_Children(children.begin(), children.end())
{
	for (auto child : children)
	{
		assert(child->m_Parent.expired() == true);
		child->m_Parent = weak_from_this();
	}
}

void UI::Group::AddChild(std::shared_ptr<Node> newChild)
{
	if (auto oldParent = newChild->m_Parent.lock())
		oldParent->RemoveChild(newChild);

	newChild->m_Parent = weak_from_this();
	m_Children.insert(newChild);
}

bool UI::Group::RemoveChild(const std::shared_ptr<Node>& child)
{
	size_t numDeleted = m_Children.erase(child);
	if (numDeleted == 1)
		child->m_Parent.reset();

	assert(numDeleted < 2);
	return numDeleted > 0;
}
