#ifndef UI_HEADER
#define UI_HEADER

#include <AT2/AT2.h>
#include <memory>
#include <set>
#include <variant>

#include <glm/glm.hpp>

namespace UI
{
	enum class Alignment
	{
		Horizontal,
		Vertical
	};

	struct CanvasData
	{
		glm::ivec2 Position; //top left point of the UI AABB
		glm::uvec2 MinimalSize; //minimal measured size
		glm::uvec2 MeasuredSize;
	};

	class Group;

	class Node
	{
		friend class Group;

	public:
		virtual ~Node() {}

	public:
		std::weak_ptr<Group> GetParent() const						{ return m_Parent; }
		std::string_view GetName() const							{ return m_Name; }

		void SetSize(const glm::ivec2& size)						{ m_Size = size; }
		const glm::ivec2& GetSize() const							{ return m_Size; }

		//recursively calculates the actual sizes of all children
		virtual void Measure(const glm::ivec2& position, const glm::uvec2& possibleSize);
		virtual glm::uvec2 ComputeMinimalSize()						{ return m_CanvasData.MinimalSize = m_Size; }
		const glm::uvec2 GetMinimalSize() const						{ return m_CanvasData.MinimalSize; }

	protected:
		Node(std::string_view name, const glm::ivec2& size) : m_Name(name), m_Size(size) {};

		CanvasData& GetCanvasData() { return m_CanvasData; }
		const CanvasData& GetCanvasData() const { return m_CanvasData; }

	protected:
		std::weak_ptr<Group> m_Parent;

	private:
		std::string m_Name;
		glm::ivec2 m_Size;

		CanvasData m_CanvasData;
	};

	class Group : public Node, public std::enable_shared_from_this<Group>
	{
	public:
		void AddChild(std::shared_ptr<Node> newChild);
		bool RemoveChild(const std::shared_ptr<Node>& child);

	protected:
		Group(std::string_view name, const glm::ivec2& size) : Node(name, size) {}
		void Initialize(std::initializer_list<std::shared_ptr<Node>> children);

	protected:
		std::set<std::shared_ptr<Node>> m_Children;
	};

	class StackPanel : public Group
	{
	protected:
		StackPanel(std::string_view name, Alignment alignment, const glm::ivec2& size) : Group(name, size), m_Alignment(alignment) {}

	public:
		

	public:
		static std::shared_ptr<StackPanel> Make(std::string_view name, Alignment alignment, std::initializer_list<std::shared_ptr<Node>> children, const glm::ivec2& size = glm::ivec2());

		void Measure(const glm::ivec2& position, const glm::uvec2& possibleSize) override;
		glm::uvec2 ComputeMinimalSize() override;

	protected:
		Alignment m_Alignment;
	};

	class Button : public Node
	{
	protected:
		Button(std::string_view name, const glm::ivec2& size) : Node(name, size) {}

	public:
		static std::shared_ptr<Button> Make(std::string_view name, const glm::ivec2& size = glm::ivec2());

		~Button() { std::cout << "Button" << std::endl; }
		
	protected:


		
	};

	class Plot : public Node
	{
	public:
		static std::shared_ptr<Plot> Make(std::string_view name, const glm::ivec2& size = glm::ivec2());

		~Plot() { std::cout << "Plot" << std::endl; }

	protected:
		Plot(std::string_view name, const glm::ivec2& size) : Node(name, size) {}

	};
}

#endif