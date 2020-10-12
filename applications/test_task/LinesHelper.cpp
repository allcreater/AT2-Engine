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

void LinesHelper::Init(const IRenderer& renderer)
{
    auto& rf = renderer.GetResourceFactory();

    m_VAO = rf.CreateVertexArray();
    m_VAO->SetVertexBuffer(0, rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer, 0, nullptr), BufferDataTypes::Vec2);
    m_VAO->SetVertexBuffer(1, rf.CreateVertexBuffer(VertexBufferType::ArrayBuffer, 0, nullptr), BufferDataTypes::Vec4);
    //m_VAO->SetIndexBuffer(rf.CreateVertexBuffer(AT2vbt::IndexBuffer, 0, nullptr), BufferDataTypes::UInt);
}

void LinesHelper::UpdateVAO(const IRenderer& renderer)
{
    if (m_VAO == nullptr)
        Init(renderer);

    m_VAO->GetVertexBuffer(0)->SetData(m_vertices.size() * sizeof(glm::vec2), m_vertices.data());
    m_VAO->GetVertexBuffer(1)->SetData(m_colors.size() * sizeof(glm::vec4), m_colors.data());
}
