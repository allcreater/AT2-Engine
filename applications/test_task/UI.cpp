#include "UI.h"

using namespace AT2;
using namespace AT2::UI;

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

		m_uniforms->SetUniform("u_matProjection", m_projectionMatrix);

		m_uniforms->Bind();
		m_DrawPrimitive->Draw();
	}

	void UpdateFromData(const std::shared_ptr<IRenderer>& renderer, const std::vector<float>& data, float x1, float x2)
	{
		if (m_VAO == nullptr)
			Init(renderer);

		m_VAO->GetVertexBuffer(0)->SetData(data.size() * sizeof(float), data.data());

		m_DrawPrimitive = std::make_unique<GlDrawArraysPrimitive>(GlDrawPrimitiveType::LineStrip, 0, data.size());

		m_uniforms->SetUniform("u_BoundsX", glm::vec2(x1, x2));
		m_uniforms->SetUniform("u_NumberOfPoints", (glm::uint32_t)data.size());
		m_uniforms->SetUniform("u_Color", glm::vec4(1.0, 1.0, 0.0, 1.0));
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

		controlPtr->EnumerateCurves([&](std::string_view name, std::vector<float> data, bool isInvalidated, std::pair<float,float> range) {
			auto emplaceResult = m_curves.try_emplace(std::string(name), std::make_shared<CurveDrawable>());
			if (isInvalidated)
				emplaceResult.first->second->UpdateFromData(renderer, data, range.first, range.second);
		});

		for (auto& pair : m_curves)
			pair.second->SetProjectionMatrix(m_projectionMatrix);

		Clear();
		AddLine(glm::vec2(observingRange.MinBound.x, 0.0), glm::vec2(observingRange.MaxBound.x, 0.0));
		AddLine(glm::vec2(0.0, observingRange.MinBound.y), glm::vec2(0.0, observingRange.MaxBound.y));
	}

	m_uniformBuffer->SetUniform("u_matProjection", m_projectionMatrix);
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
