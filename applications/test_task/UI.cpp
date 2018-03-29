#include "UI.h"

using namespace AT2;
using namespace AT2::UI;

//TODO: hide from the interface!
#include <AT2/OpenGl/GlDrawPrimitive.h>


//it is possible to contain all data in one vertex buffer array and one vertex buffer, but it's more complex task and much unclear code, so let's just draw it separately
class CurveDrawable : public IDrawable
{
	void Draw(const std::shared_ptr<IRenderer>& renderer) override
	{
		auto& stateManager = renderer->GetStateManager();

		stateManager.BindVertexArray(m_VAO);
		//stateManager.BindTextures(Textures);

		m_uniforms->Bind();
		m_DrawPrimitive->Draw();
	}

	void UpdateFromData(const std::shared_ptr<IRenderer>& renderer, const std::vector<float>& data, float x1, float x2)
	{
		auto& rf = renderer->GetResourceFactory();

		m_VAO = rf.CreateVertexArray();
		m_VAO->SetVertexBuffer(0, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec3, data.size() * sizeof(glm::vec3), data.data()));

		m_DrawPrimitive = std::make_unique<GlDrawArraysPrimitive>(GlDrawPrimitiveType::LineStrip, 0, data.size());
		

		m_uniforms->SetUniform("u_BoundsX", glm::vec2(x1, x2));
		m_uniforms->SetUniform("u_NumberOfPoints", (glm::uint32_t)data.size());
	}

private:
	std::shared_ptr<IVertexArray> m_VAO;
	std::unique_ptr<IDrawPrimitive> m_DrawPrimitive;
	std::shared_ptr<IUniformContainer> m_uniforms;
};



void PlotRenderer::Draw(const std::shared_ptr<IRenderer>& renderer)
{
	if (m_uiVAO == nullptr)
		Init(renderer);

	auto& stateManager = renderer->GetStateManager();

	stateManager.BindShader(m_uiShader);
	//stateManager.BindTextures(Textures);
	stateManager.BindVertexArray(m_uiVAO);
		

	m_uniformBuffer->Bind();

	for (auto primitive : m_uiPrimitiveList)
		primitive->Draw();


}

void PlotRenderer::PrepareData(const std::shared_ptr<IRenderer>& renderer)
{

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
		

	glm::vec2 positions[] = { glm::vec2(-1.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(1.0, 1.0), glm::vec2(-1.0, 1.0) };
	glm::vec4 colors[] = { glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,0,0,1), glm::vec4(1,1,1,1) };
	glm::uint indices[] = { 0, 2, 1, 3 };

	auto& rf = renderer->GetResourceFactory();

	m_uiVAO = rf.CreateVertexArray();
	m_uiVAO->SetVertexBuffer(0, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec2, 4 * sizeof(glm::vec2), positions));
	m_uiVAO->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, AT2::BufferDataTypes::Vec4, 4 * sizeof(glm::vec4), colors));
	m_uiVAO->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, AT2::BufferDataTypes::UInt, 4 * sizeof(glm::uint), indices));
	m_uiPrimitiveList.push_back(new AT2::GlDrawElementsPrimitive(AT2::GlDrawPrimitiveType::Lines, 4, AT2::GlDrawElementsPrimitive::IndicesType::UnsignedInt, 0));
}
