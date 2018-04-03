#include "UI.h"

using namespace AT2;
using namespace AT2::UI;

#include <algorithm>

//TODO: hide from the interface!
#include <AT2/OpenGl/GlDrawPrimitive.h>


//it is possible to contain all data in one vertex buffer array and one vertex buffer, but it's more complex task and much unclear code, so let's just draw it separately
class AT2::UI::CurveDrawable : public IDrawable
{
public:
	CurveDrawable()
	{
	}

	void Draw(const std::shared_ptr<IRenderer>& renderer) override
	{
		if (m_VAO == nullptr)
			Init(renderer);

		auto& stateManager = renderer->GetStateManager();
		stateManager.BindVertexArray(m_VAO);
		glLineWidth(1.5);
		m_uniforms->SetUniform("u_matProjection", m_projectionMatrix);

		m_uniforms->Bind();
		m_DrawPrimitive->Draw();
	}

	void RebuildFromData(const std::shared_ptr<IRenderer>& renderer, const Plot::CurveData& data)
	{
		if (m_VAO == nullptr)
			Init(renderer);

		m_VAO->GetVertexBuffer(0)->SetData(data.Data.size() * sizeof(float), data.Data.data()); //data.data.data :)

		m_DrawPrimitive = std::make_unique<GlDrawArraysPrimitive>(GlDrawPrimitiveType::LineStrip, 0, data.Data.size());

		m_uniforms->SetUniform("u_BoundsX", glm::vec2(data.GetCurveBounds().MinBound.x, data.GetCurveBounds().MaxBound.x));
		m_uniforms->SetUniform("u_NumberOfPoints", (glm::uint32_t)data.Data.size());
		m_uniforms->SetUniform("u_Color", data.GetColor());
	}

	void RefreshFromData(const std::shared_ptr<IRenderer>& renderer, const Plot::CurveData& data)
	{
		if (m_VAO == nullptr)
			throw AT2::AT2Exception("AT2::UI::CurveDrawable should be created before refreshing!");

		m_uniforms->SetUniform("u_BoundsX", glm::vec2(data.GetCurveBounds().MinBound.x, data.GetCurveBounds().MaxBound.x));
		m_uniforms->SetUniform("u_Color", data.GetColor());
	}

	void SetProjectionMatrix(const glm::mat4& matProj)
	{
		m_projectionMatrix = matProj;
	}
private:
	void Init(const std::shared_ptr<IRenderer>& renderer)
	{
		auto& rf = renderer->GetResourceFactory();

		m_VAO = rf.CreateVertexArray();
		m_VAO->SetVertexBuffer(0, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Float, 0, 0));


		m_uniforms = renderer->GetStateManager().GetActiveShader().lock()->CreateAssociatedUniformStorage();
	}

private:
	std::shared_ptr<IVertexArray> m_VAO;
	std::unique_ptr<IDrawPrimitive> m_DrawPrimitive;
	std::shared_ptr<IUniformContainer> m_uniforms;
	glm::mat4 m_projectionMatrix;
};



void PlotRenderer::Draw(const std::shared_ptr<IRenderer>& renderer)
{
	if (m_uiShader == nullptr)
		Init(renderer);

	auto& stateManager = renderer->GetStateManager();

	stateManager.BindShader(m_uiShader);
	m_uniformBuffer->Bind();
	LinesHelper::Draw(renderer);


	stateManager.BindShader(m_curveShader);
	PrepareData(renderer);

	for (auto pair : m_curves)
		pair.second->Draw(renderer);

}

void PlotRenderer::PrepareData(const std::shared_ptr<IRenderer>& renderer)
{
	if (auto controlPtr = m_Control.lock())
	{
		auto& observingRange = controlPtr->GetObservingZone();
		m_projectionMatrix = glm::ortho(observingRange.MinBound.x, observingRange.MaxBound.x, observingRange.MaxBound.y, observingRange.MinBound.y);

		controlPtr->EnumerateCurves([&](const std::string_view name, const Plot::CurveData& data, bool isInvalidated)
		{
			auto emplaceResult = m_curves.try_emplace(std::string(name), std::make_shared<CurveDrawable>());
			if (isInvalidated || emplaceResult.second)
				emplaceResult.first->second->RebuildFromData(renderer, data);
			else
				emplaceResult.first->second->RefreshFromData(renderer, data);

			return true;
		});

		for (auto& pair : m_curves)
			pair.second->SetProjectionMatrix(m_projectionMatrix);

		UpdateCanvasGeometry(observingRange);
	}

	m_uniformBuffer->SetUniform("u_matProjection", m_projectionMatrix);
}

void PlotRenderer::UpdateCanvasGeometry(const AABB2d& observingRange)
{
	Clear();
	AddLine(glm::vec2(observingRange.MinBound.x, 0.0), glm::vec2(observingRange.MaxBound.x, 0.0));
	AddLine(glm::vec2(0.0, observingRange.MinBound.y), glm::vec2(0.0, observingRange.MaxBound.y));

	glm::vec2 range = observingRange.MaxBound - observingRange.MinBound;
	glm::vec2 exp = glm::floor(glm::log(range)/ glm::log(decltype(range)(10.0f)));

	glm::vec2 minExp = glm::floor(glm::log(observingRange.MaxBound) / glm::log(decltype(range)(10.0f)));

	AddLine(glm::vec2(pow(10, minExp.x), observingRange.MinBound.y), glm::vec2(pow(10, minExp.x), observingRange.MaxBound.y), glm::vec4(1.0,1.0,1.0, 0.3));
	//AddLine(glm::vec2(minExp.x * 10, 0.0), glm::vec2(minExp.x * 10, 0.0));
}

void PlotRenderer::Init(const std::shared_ptr<IRenderer>& renderer)
{
	m_uiShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
		{
			"resources//shaders//simple.vs.glsl",
			"resources//shaders//simple.fs.glsl"
		});

	m_curveShader = renderer->GetResourceFactory().CreateShaderProgramFromFiles(
		{
			"resources//shaders//curve.vs.glsl",
			"resources//shaders//curve.fs.glsl"
		});
		

	std::vector<glm::vec2> positions = {glm::vec2()};
	std::vector<glm::vec4> colors;

	m_uniformBuffer = m_uiShader->CreateAssociatedUniformStorage();
}


//
// UiInputHandler
//

void UiInputHandler::OnMouseMove(const MousePos& mousePos)
{
	m_mousePos = mousePos;

	bool eventCatched = false;

	m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
		if (!eventCatched && isPointInsideNode(node, m_mousePos.getPos()))
		{
			if (auto& vector = m_mouseDownOnControl[0]; std::find_if(vector.begin(), vector.end(), [&](const std::weak_ptr<Node>& n) {return n.lock() == node; }) != vector.end())
				if (EventClicked)
					eventCatched |= EventMouseDrag(node, m_mousePos);
		}
	});

}

void UiInputHandler::OnMouseDown(int key)
{
	m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
		if (isPointInsideNode(node, m_mousePos.getPos()))
			m_mouseDownOnControl[key].push_back(node);
	});

}
void UiInputHandler::OnMouseUp(int key)
{
	bool eventCatched = false;

	if (key == 0)
	{
		m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
			if (!eventCatched && isPointInsideNode(node, m_mousePos.getPos()))
			{
				if (auto& vector = m_mouseDownOnControl[key]; std::find_if(vector.begin(), vector.end(), [&](const std::weak_ptr<Node>& n) {return n.lock() == node; }) != vector.end())
					if (EventClicked)
						eventCatched |= EventClicked(node);
			}
		});
	}

	m_mouseDownOnControl[key].clear();
}

void UiInputHandler::OnMouseScroll(const glm::vec2& scrollDir)
{
	bool eventCatched = false;

	m_rootNode->TraverseDepthFirst([&](std::shared_ptr<Node>& node) {
		if (!eventCatched && isPointInsideNode(node, m_mousePos.getPos()))
		{
			if (EventScrolled)
				eventCatched |= EventScrolled(node, m_mousePos, scrollDir);

			eventCatched = true;
		}
	});
}

bool UiInputHandler::isPointInsideNode(std::shared_ptr<Node>& node, const glm::vec2& pos)
{
	return node->GetScreenPosition().IsPointInside(pos);
}