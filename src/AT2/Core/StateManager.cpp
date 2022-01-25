#include "StateManager.h"

#include <cassert>

using namespace AT2;

StateManager::StateManager(IVisualizationSystem& renderer)
    : m_renderer(renderer)
{
}

void StateManager::BindShader(const std::shared_ptr<IShaderProgram>& _shader)
{
    assert(_shader);

    if (m_activeShader && m_activeShader == _shader)
        return;

    DoBind(*_shader);
    m_activeShader = _shader;
}

void StateManager::BindVertexArray(const std::shared_ptr<IVertexArray>& _vertexArray)
{
    if (m_activeVertexArray && m_activeVertexArray == _vertexArray)
        return;

    if (_vertexArray)
    {
        DoBind(*_vertexArray);

        m_activeIndexBufferType = _vertexArray->GetIndexBufferType();
    }

    m_activeVertexArray = _vertexArray;
}

//TextureSet& StateManager::GetActiveTextures() const
//{
//	throw AT2::AT2Exception("Not implemented yet :(");
//}

std::shared_ptr<IShaderProgram> StateManager::GetActiveShader() const
{
    return m_activeShader;
}

std::shared_ptr<IVertexArray> StateManager::GetActiveVertexArray() const
{
    return m_activeVertexArray;
}
