#include "UI.h"

#include <algorithm>

using namespace AT2::UI;

void StackPanel::Measure(const glm::ivec2& position, const glm::uvec2& possibleSize)
{
	Group::Measure(position, possibleSize);
	//now actual position is GetCanvasData().Position and size is GetCanvasData().MeasuredSize

	//TODO: it should take decision about auto size by Stretch align, not by minimal size
	typedef const std::function<bool(std::shared_ptr<Node> nodePtr)> Func;
	Func horizontalFunc = [](const std::shared_ptr<Node> &nodePtr) { return nodePtr->GetMinimalSize().x == 0; };
	Func verticalFunc = [](const std::shared_ptr<Node> &nodePtr) { return nodePtr->GetMinimalSize().y == 0; };
	Func isAutoSizedFunc = (m_Orientation == Orientation::Horizontal) ? horizontalFunc : verticalFunc;

	const unsigned int numberOfAutoSizedChildren = std::count_if(m_Children.begin(), m_Children.end(), isAutoSizedFunc);

	//numberOfAutoSizedChildren == 0 is special case when elementCharacteristicalSize don't using at all, so it could be random, just not division by zero :)
	const auto elementCharacteristicalSize = (GetCanvasData().MeasuredSize - GetMinimalSize()) / std::max(numberOfAutoSizedChildren, 1u);
	glm::ivec2 currentPosition = GetCanvasData().Position;
	for (const auto& child : m_Children)
	{
		glm::uvec2 measuringSize = isAutoSizedFunc(child) ? elementCharacteristicalSize : child->GetMinimalSize();
		if (m_Orientation == Orientation::Horizontal)
			measuringSize.y = GetCanvasData().MeasuredSize.y;
		else
			measuringSize.x = GetCanvasData().MeasuredSize.x;

		child->Measure(currentPosition, measuringSize);

		//TODO: remove this pretty ugly code :(
		if (m_Orientation == Orientation::Horizontal)
			currentPosition.x += measuringSize.x;
		else
			currentPosition.y += measuringSize.y;
	}
}

glm::uvec2 StackPanel::ComputeMinimalSize()
{
	if (m_Children.empty())
		return GetSize();

	//oppa functional-style!
	typedef const std::function<glm::ivec2(glm::ivec2&, const glm::ivec2&)> Func;
	Func horizontalFunc = [](glm::ivec2& accum, const glm::ivec2& size) { return glm::vec2(accum.x += size.x, std::max(accum.y, size.y)); };
	Func verticalFunc = [](glm::ivec2& accum, const glm::ivec2& size) { return glm::vec2(std::max(accum.x, size.x), accum.y += size.y); };
	Func aggregateFunc = (m_Orientation == Orientation::Horizontal) ? horizontalFunc : verticalFunc;


	glm::ivec2 minimalContentSize = {};
	for (auto& child : m_Children)
		minimalContentSize = aggregateFunc(minimalContentSize, child->ComputeMinimalSize());

	assert(minimalContentSize.x >= 0 && minimalContentSize.y >= 0); //TODO: decide how to interpret negative values, or just forbid them :)
	return GetCanvasData().MinimalSize = minimalContentSize;
}
