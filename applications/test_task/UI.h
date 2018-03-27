#ifndef UI_HEADER
#define UI_HEADER

#include <AT2/AT2.h>
#include <memory>
#include <set>
#include <optional>

#include <glm/glm.hpp>

namespace UI
{
	enum class Alignment
	{
		Horizontal,
		Vertical
	};

	class Group;

	class Node
	{
		friend class Group;

	public:
		std::weak_ptr<Group> GetParent() const { return m_Parent; }

	protected:
		std::weak_ptr<Group> m_Parent;
	};

	class Group : public Node, public std::enable_shared_from_this<Group>
	{
	protected:
		Group(std::initializer_list<Node*> children);

	public:
		void AddChild(std::shared_ptr<Node> newChild);
		bool RemoveChild(const std::shared_ptr<Node>& child);

	protected:
		std::set<std::shared_ptr<Node>> m_Children;
	};

	class StackPanel : public Group
	{
	public:
		StackPanel(Alignment alignment, std::initializer_list<Node*> children) : Group(children) {}


	};

	class Button : public Node
	{
		
	};

	class Plot : public Node
	{

	};
}

#endif