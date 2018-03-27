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
		virtual ~Node() {}

	public:
		std::weak_ptr<Group> GetParent() const { return m_Parent; }
		std::string_view GetName() const { return m_Name; }

	protected:
		Node(std::string_view name) : m_Name(name) {};

	protected:
		std::weak_ptr<Group> m_Parent;
		std::string m_Name;
	};

	class Group : public Node, public std::enable_shared_from_this<Group>
	{
	public:
		void AddChild(std::shared_ptr<Node> newChild);
		bool RemoveChild(const std::shared_ptr<Node>& child);

	protected:
		Group(std::string_view name) : Node(name) {}
		void Initialize(std::initializer_list<std::shared_ptr<Node>> children);

	protected:
		std::set<std::shared_ptr<Node>> m_Children;
	};

	class StackPanel : public Group
	{
	protected:
		StackPanel(std::string_view name, Alignment alignment) : Group(name), m_Alignment(alignment) {}

	public:
		~StackPanel() { std::cout << "StackPanel" << std::endl; }

	public:
		static std::shared_ptr<StackPanel> Make(std::string_view name, Alignment alignment, std::initializer_list<std::shared_ptr<Node>> children)
		{
			auto ptr = std::shared_ptr<StackPanel>(new StackPanel(name, alignment));
			ptr->Initialize(children);
			return ptr;
		}
	protected:
		Alignment m_Alignment;
	};

	class Button : public Node
	{
	protected:
		Button(std::string_view name) : Node(name) {}

	public:
		static std::shared_ptr<Button> Make(std::string_view name)
		{
			return std::shared_ptr<Button>(new Button(name));
		}

		~Button() { std::cout << "Button" << std::endl; }
		
	protected:


		
	};

	class Plot : public Node
	{
	public:
		static std::shared_ptr<Plot> Make(std::string_view name)
		{
			return std::shared_ptr<Plot>(new Plot(name));
		}

		~Plot() { std::cout << "Plot" << std::endl; }

	protected:
		Plot(std::string_view name) : Node(name) {}

	};
}

#endif