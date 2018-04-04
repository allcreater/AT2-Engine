#include "LinesHelper.h"

#include <AT2/OpenGL/GlDrawPrimitive.h>

using namespace AT2;

LinesHelper::LinesHelper()
{

}

void LinesHelper::Draw(const std::shared_ptr<IRenderer>& renderer)
{
	if (m_vertexBufferNeedUpdate)
	{
		UpdateVAO(renderer);
		m_vertexBufferNeedUpdate = false;
	}

	auto& stateManager = renderer->GetStateManager();
	stateManager.BindVertexArray(m_VAO);
	m_DrawPrimitive->Draw();
}

void LinesHelper::AddLine(const glm::vec2& begin, const glm::vec2& end, const glm::vec4& color)
{
	m_vertices.emplace_back(begin);
	m_colors.emplace_back(color);

	m_vertices.emplace_back(end);
	m_colors.emplace_back(color);

	m_vertexBufferNeedUpdate = true;
}

void LinesHelper::Clear()
{
	m_vertices.resize(0);
	m_colors.resize(0);

	m_vertexBufferNeedUpdate = true;
}

void LinesHelper::Init(const std::shared_ptr<IRenderer>& renderer)
{
	auto& rf = renderer->GetResourceFactory();

	m_VAO = rf.CreateVertexArray();
	m_VAO->SetVertexBuffer(0, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, BufferDataTypes::Vec2, 0, nullptr));
	m_VAO->SetVertexBuffer(1, rf.CreateVertexBuffer(AT2vbt::ArrayBuffer, BufferDataTypes::Vec4, 0, nullptr));
	//m_VAO->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, BufferDataTypes::UInt, 0, nullptr));
}

void LinesHelper::UpdateVAO(const std::shared_ptr<IRenderer>& renderer)
{
	if (m_VAO == nullptr)
		Init(renderer);

	m_VAO->GetVertexBuffer(0)->SetData(m_vertices.size() * sizeof(glm::vec2), m_vertices.data());
	m_VAO->GetVertexBuffer(1)->SetData(m_colors.size() * sizeof(glm::vec4), m_colors.data());

	m_DrawPrimitive = std::make_unique<GlDrawArraysPrimitive>(GlDrawPrimitiveType::Lines, 0, m_vertices.size());
}
