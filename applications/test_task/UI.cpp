#include "UI.h"

using namespace UI;

//TODO: write unit tests

void Group::Initialize(std::initializer_list<std::shared_ptr<Node>> children)
{
	assert(m_Children.empty());
	m_Children = std::set<std::shared_ptr<Node>>(children.begin(), children.end());

	for (auto child : children)
	{
		assert(child->m_Parent.expired() == true);
		child->m_Parent = weak_from_this();
	}
}

void Group::AddChild(std::shared_ptr<Node> newChild)
{
	if (auto oldParent = newChild->m_Parent.lock())
		oldParent->RemoveChild(newChild);

	newChild->m_Parent = weak_from_this();
	m_Children.insert(newChild);
}

bool Group::RemoveChild(const std::shared_ptr<Node>& child)
{
	size_t numDeleted = m_Children.erase(child);
	if (numDeleted == 1)
		child->m_Parent.reset();

	assert(numDeleted < 2);
	return numDeleted > 0;
}
