#include "UI.h"

#include <algorithm>

using namespace AT2::UI;

//TODO: write unit tests

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

void Node::Measure(const glm::ivec2 & position, const glm::uvec2 & possibleSize)
{
	m_CanvasData.MeasuredSize = possibleSize;
	m_CanvasData.Position = position;
}
