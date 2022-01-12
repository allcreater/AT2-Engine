#include "StateManager.h"
#include <algorithm>
#include <cassert>
#include <numeric>

using namespace AT2;

StateManager::StateManager(IVisualizationSystem& renderer)
    : m_renderer(renderer)
	, m_freeTextureSlots(renderer.GetRendererCapabilities().GetMaxNumberOfTextureUnits())
    , m_activeTextures(renderer.GetRendererCapabilities().GetMaxNumberOfTextureUnits())
{
    std::iota(m_freeTextureSlots.begin(), m_freeTextureSlots.end(), 0);
    std::reverse(m_freeTextureSlots.begin(), m_freeTextureSlots.end());
}

void StateManager::BindTextures(const TextureSet& _textures)
{
    //TODO: use Strategy pattern
    //TODO: release textures with reference count == 1
    const auto texturesMapper = [this](const std::shared_ptr<const ITexture>& texture) {
        assert(!m_freeTextureSlots.empty());

        const auto textureIndex = m_freeTextureSlots.back();
        DoBind(*texture, textureIndex);

        m_freeTextureSlots.pop_back();
        return std::tuple {textureIndex};
    };

    const auto textureUnmapper = [this](auto&& kv) { m_freeTextureSlots.push_back(kv.second); };

    for (const auto& texture : _textures)
        m_activeTextures.put(texture, texturesMapper, textureUnmapper);
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

std::optional<unsigned> StateManager::GetActiveTextureIndex(std::shared_ptr<const ITexture> texture) const noexcept
{
    return m_activeTextures.find(texture);
}
