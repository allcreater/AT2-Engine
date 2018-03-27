#include "UI.h"

#include <algorithm>

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


void StackPanel::Measure(const glm::ivec2& position, const glm::uvec2& possibleSize)
{
	typedef const std::function<bool(std::shared_ptr<Node> nodePtr)> Func;
	Func horizontalFunc =	[](std::shared_ptr<Node> nodePtr) { return nodePtr->GetMinimalSize().x == 0; };
	Func verticalFunc =		[](std::shared_ptr<Node> nodePtr) { return nodePtr->GetMinimalSize().y == 0; };
	Func isAutoSizedFunc = (m_Alignment == Alignment::Horizontal) ? horizontalFunc : verticalFunc;

	size_t numberOfAutoSizedChildren = std::count_if(m_Children.begin(), m_Children.end(), isAutoSizedFunc);

	//numberOfAutoSizedChildren == 0 is special case when elementCharacteristicalSize don't using at all, so it could be random, just not division by zero :)
	auto elementCharacteristicalSize = (possibleSize - GetMinimalSize()) / std::max(numberOfAutoSizedChildren, 1u);
	glm::ivec2 currentPosition = position;
	for (auto child : m_Children)
	{
		glm::uvec2 measuringSize = isAutoSizedFunc(child) ? elementCharacteristicalSize : child->GetMinimalSize();
		if (m_Alignment == Alignment::Horizontal)
			measuringSize.y = possibleSize.y;
		else
			measuringSize.x = possibleSize.x;

		child->Measure(currentPosition, measuringSize);

		//TODO: remove this preety ugly code :(
		if (m_Alignment == Alignment::Horizontal)
			currentPosition.x += measuringSize.x;
		else
			currentPosition.y += measuringSize.y;
	}

	Node::Measure(position, GetMinimalSize()); //or we could use all possible space
}

glm::uvec2 StackPanel::ComputeMinimalSize()
{
	if (m_Children.empty())
		return GetSize();

	//oppa functional-style!
	typedef const std::function<glm::ivec2(glm::ivec2&, const glm::ivec2&)> Func;
	Func horizontalFunc =	[](glm::ivec2& accum, const glm::ivec2& size) { return glm::vec2(accum.x += size.x, std::max(accum.y, size.y)); };
	Func verticalFunc =		[](glm::ivec2& accum, const glm::ivec2& size) { return glm::vec2(std::max(accum.x, size.x), accum.y += size.y); };
	Func aggregateFunc = (m_Alignment == Alignment::Horizontal) ? horizontalFunc : verticalFunc;

	
	glm::ivec2 minimalContentSize;
	for (auto child : m_Children)
		minimalContentSize = aggregateFunc(minimalContentSize, child->ComputeMinimalSize());

	assert(minimalContentSize.x >= 0 && minimalContentSize.y >= 0); //TODO: decide how to interpret negative values, or just forbid them :)
	return GetCanvasData().MinimalSize = minimalContentSize;
}

// Constructor methods
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

//recursively calculates the actual sizes of all children

inline void UI::Node::Measure(const glm::ivec2 & position, const glm::uvec2 & possibleSize)
{
	m_CanvasData.MeasuredSize = possibleSize;
	m_CanvasData.Position = position;
}
