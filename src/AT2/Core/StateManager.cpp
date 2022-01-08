#include "StateManager.h"
#include <algorithm>
#include <cassert>

using namespace AT2;

StateManager::StateManager(IRenderer& renderer)
    : m_renderer(renderer)
	, m_bindedTextures(renderer.GetRendererCapabilities().GetMaxNumberOfTextureUnits())
{
}

void StateManager::BindTextures(const TextureSet& _textures)
{
    const auto numModules = m_bindedTextures.size();
    if (_textures.size() > numModules)
        throw AT2Exception("StateManager: trying to bind more textures than free texture units");

    //let's mark texture units which stay unchanged
    std::vector<bool> moduleLock(m_bindedTextures.size());
    for (const auto& texture : _textures)
    {
        if (const auto unit = GetActiveTextureIndex(texture))
            moduleLock[*unit] = true;
    }


    auto textureToBoundIterator = _textures.begin();
    for (unsigned currentModule = 0; currentModule < numModules && textureToBoundIterator != _textures.end();
         ++currentModule)
    {
        if (!GetActiveTextureIndex(*textureToBoundIterator))
        {
            if (!moduleLock[currentModule])
            {
                auto& texture = m_bindedTextures[currentModule];

                texture = *textureToBoundIterator;
                DoBind(*texture, currentModule);

                ++textureToBoundIterator;
            }
        }
        else
            ++textureToBoundIterator;
    }
}

void StateManager::BindFramebuffer(const std::shared_ptr<IFrameBuffer>& _framebuffer)
{
    if (m_activeFramebuffer == _framebuffer)
        return;

    DoBind(_framebuffer ? *_framebuffer : m_renderer.GetDefaultFramebuffer());

    m_activeFramebuffer = _framebuffer;
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

std::shared_ptr<IFrameBuffer> StateManager::GetActiveFrameBuffer() const
{
    return m_activeFramebuffer;
}

std::shared_ptr<IShaderProgram> StateManager::GetActiveShader() const
{
    return m_activeShader;
}

std::shared_ptr<IVertexArray> StateManager::GetActiveVertexArray() const
{
    return m_activeVertexArray;
}

std::optional<unsigned> StateManager::GetActiveTextureIndex( std::shared_ptr<const ITexture> texture ) const noexcept
{
    auto it = std::find(m_bindedTextures.begin(), m_bindedTextures.end(), texture);
    return it != m_bindedTextures.end() ? std::distance(m_bindedTextures.begin(), it) : std::optional<unsigned> {};
}
