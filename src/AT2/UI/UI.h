#ifndef UI_HEADER
#define UI_HEADER

#include "../Drawable.h"
#include <memory>
#include <set>
#include <variant>
#include <map>

#include <glm/glm.hpp>

//This is an attempt to implement simple tree-structured extensible UI with automatic layout
//Unfortunately elements aligning is not fully supported, it will be fixed later

//TODO: use lazy size calculation instead of direct ComputeMinimalSize() call

namespace AT2::UI
{
	struct CanvasData
	{
		glm::ivec2 Position; //top left point of the UI AABB
		glm::uvec2 MinimalSize; //minimal measured size
		glm::uvec2 MeasuredSize;
	};

	class Node;
	class Group;
	class StackPanel;
	class Button;
	class Plot;

	//classic visitor with classic coupling problems, probably should be replaced for approach with dynamic_cast instead of double dispatcherisation
	struct Visitor
	{
		virtual void Visit(Node& ref) = 0;
		virtual void Visit(Group& ref) = 0;
		virtual void Visit(StackPanel& ref) = 0;
		virtual void Visit(Button& ref) = 0;
		virtual void Visit(Plot& ref) = 0;
	};

	enum class Alignment
	{
		Side1,
		Center,
		Stretch,
		Side2
	};

	enum class Orientation
	{
		Horizontal,
		Vertical
	};

	class Node
	{
		friend class Group;
		
	public:
		virtual ~Node() = default;

	public:
		Alignment VerticalAlignment = Alignment::Stretch;
		Alignment HorizontalAlignment = Alignment::Stretch;

	public:
		std::weak_ptr<Group> GetParent() const						{ return m_Parent; }
		std::string_view GetName() const							{ return m_Name; }

		void SetSize(const glm::ivec2& size)						{ m_Size = size; }
		const glm::ivec2& GetSize() const							{ return m_Size; }

		//recursively calculates the actual sizes of all children
		virtual void Measure(const glm::ivec2& position, const glm::uvec2& possibleSize);
		virtual glm::uvec2 ComputeMinimalSize()						{ return m_CanvasData.MinimalSize = m_Size; }
		const glm::uvec2 GetMinimalSize() const						{ return m_CanvasData.MinimalSize; }

		CanvasData& GetCanvasData()									{ return m_CanvasData; }
		const CanvasData& GetCanvasData() const						{ return m_CanvasData; }

		void SetNodeRenderer(std::shared_ptr<IDrawable> drawable)	{ m_Drawable = drawable; }
		std::weak_ptr<IDrawable> GetNodeRenderer() const			{ return m_Drawable; }

		virtual void Accept(Visitor& visitor)						{ visitor.Visit(*this); }

	public: //events
		std::function<bool(const Node& node)> EventClicked;
		std::function<bool(const Node& node)> EventMouseDrag;

	protected:
		Node(std::string_view name, const glm::uvec2& size) : m_Name(name), m_Size(size) {};

	protected:
		std::weak_ptr<Group> m_Parent;

		std::string m_Name;
		glm::uvec2 m_Size;

		CanvasData m_CanvasData;

		std::shared_ptr<IDrawable> m_Drawable;
	};


	class Group : public Node, public std::enable_shared_from_this<Group>
	{
	public:
		void AddChild(std::shared_ptr<Node> newChild);
		bool RemoveChild(const std::shared_ptr<Node>& child);
		void ForEachChild(std::function<void(std::shared_ptr<Node>&)>);

		void Accept(Visitor& visitor) override { visitor.Visit(*this); }

	protected:
		Group(std::string_view name, const glm::uvec2& size) : Node(name, size) {}
		void Initialize(std::initializer_list<std::shared_ptr<Node>> children);

	protected:
		//probably std::set is best candidate with built-in duplicate protection etc, but it's not so convenient for element ordering
		//std::set<std::shared_ptr<Node>> m_Children;
		std::vector<std::shared_ptr<Node>> m_Children; 
	};


	class StackPanel : public Group
	{
	public:
		static std::shared_ptr<StackPanel> Make(std::string_view name, Orientation alignment, std::initializer_list<std::shared_ptr<Node>> children, const glm::uvec2& size = glm::uvec2());

	public:
		void Measure(const glm::ivec2& position, const glm::uvec2& possibleSize) override;
		glm::uvec2 ComputeMinimalSize() override;

		void Accept(Visitor& visitor) override { visitor.Visit(*this); }

	protected:
		StackPanel(std::string_view name, Orientation alignment, const glm::ivec2& size) : Group(name, size), m_Orientation(alignment) {}

	protected:
		Orientation m_Orientation;
	};


	class Button : public Node
	{

	public:
		static std::shared_ptr<Button> Make(std::string_view name, const glm::uvec2& size = glm::uvec2(), Alignment vertical = Alignment::Stretch, Alignment horizontal = Alignment::Stretch);

		~Button() { std::cout << "Button" << std::endl; }

	public:
		void Accept(Visitor& visitor) override { visitor.Visit(*this); }

	protected:
		Button(std::string_view name, const glm::uvec2& size) : Node(name, size) {}


	protected:


		
	};

	//TODO: is it possible to move enable_from_this to Node?
	class Plot : public Node, public std::enable_shared_from_this<Plot> 
	{
	public:
		class CurveData
		{
			friend class Plot;

		public:
			CurveData(std::weak_ptr<Plot> plot) : m_parent(plot) {}

			CurveData(const CurveData&) = delete;
			CurveData(CurveData&&) = default;
			CurveData& operator=(const CurveData&) const = delete;

		public:
			std::vector<float> Data;
			
			void SetXRange(float startX, float endX) { m_aabb.MinBound.x = startX; m_aabb.MaxBound.x = endX; }
			const AABB2d& GetCurveBounds();
			void Dirty() noexcept;

		private:
			bool m_dirtyFlag = true;
			bool m_rangeNeedsUpdate = true;
			AABB2d m_aabb;
			std::weak_ptr<Plot> m_parent;
		};

	public:
		static std::shared_ptr<Plot> Make(std::string_view name, const glm::uvec2& size = glm::uvec2());

		~Plot() { std::cout << "Plot" << std::endl; }

	public:
		//TODO: encapsulate function parameters at special class
		size_t EnumerateCurves(std::function<void(const std::string_view, const std::vector<float>&, bool, std::pair<float, float>)>);
		CurveData& GetOrCreateCurve (const std::string& curveName);

		const AABB2d& GetAABB();

		void SetObservingZone(const AABB2d& zone) { m_observingZone = zone; }
		const AABB2d& GetObservingZone() const { return m_observingZone; }

		void DirtyCurves() { m_boundsShouldBeRecalculated = true; }

		void Accept(Visitor& visitor) override { visitor.Visit(*this); }

	protected:
		Plot(std::string_view name, const glm::uvec2& size) : Node(name, size) {}

		void ComputeAABB();

	protected:
		std::map<std::string, CurveData> m_curvesData;
		bool m_boundsShouldBeRecalculated = true;
		AABB2d m_allBounds, m_observingZone;
	};



	class UiVisitor : public Visitor
	{
	public:
		void Visit(Node& node) override
		{
		}
		void Visit(Group& node) override
		{
			TraverseChildren(node);
		}
		void Visit(StackPanel& node) override
		{
			TraverseChildren(node);
		}
		void Visit(Button& node) override
		{
		}
		void Visit(Plot& node) override
		{
		}

	protected:
		void TraverseChildren(Group& group)
		{
			group.ForEachChild([this](std::shared_ptr<Node>& nodePtr) {nodePtr->Accept(*this); });
		}
	};

}
#endif