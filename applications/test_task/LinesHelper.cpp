#include "LinesHelper.h"


using namespace AT2;

void LinesHelper::Draw(IRenderer& renderer)
{
    if (m_vertexBufferNeedUpdate)
    {
        UpdateVAO(renderer);
        m_vertexBufferNeedUpdate = false;
    }

    auto& stateManager = renderer.GetStateManager();
    stateManager.BindVertexArray(m_VAO);

    renderer.Draw(Primitives::Lines{}, 0, static_cast<long>(m_vertices.size()));
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

void LinesHelper::Init(const IVisualizationSystem& renderer)
{
    auto& rf = renderer.GetResourceFactory();

    m_VAO = rf.CreateVertexArray();
    m_VAO->SetAttributeBinding(0, rf.CreateBuffer(VertexBufferType::ArrayBuffer), BufferDataTypes::Vec2);
    m_VAO->SetAttributeBinding(1, rf.CreateBuffer(VertexBufferType::ArrayBuffer), BufferDataTypes::Vec4);
    //m_VAO->SetIndexBuffer(rf.CreateBuffer(AT2vbt::IndexBuffer, 0, nullptr), BufferDataTypes::UInt);
}

void LinesHelper::UpdateVAO(const IVisualizationSystem& renderer)
{
    if (m_VAO == nullptr)
        Init(renderer);

    m_VAO->GetVertexBuffer(0)->SetData(m_vertices);
    m_VAO->GetVertexBuffer(1)->SetData(m_colors);
}
