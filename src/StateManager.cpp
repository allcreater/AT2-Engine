#include "StateManager.h"
#include <algorithm>
#include <assert.h>

using namespace AT2;

StateManager::StateManager(const IRendererCapabilities* rendererCapabilities)
	: m_bindedTextures(rendererCapabilities->GetMaxNumberOfTextureUnits())
{
}

void StateManager::BindTextures(const TextureSet& _textures)
{
	const auto numModules = m_bindedTextures.size();
	if (_textures.size() > numModules)
		throw AT2Exception(AT2Exception::ErrorCase::Unknown, "StateManager: trying to bind more textures than free texture units");

	//let's mark texture units which stay unchanged
	std::vector<bool> moduleLock(m_bindedTextures.size());
	for (const auto& texture : _textures)
	{
		const int module = texture->GetCurrentModule();
		if (module >= 0)
			moduleLock[module] = true;
	}
	

	size_t currentModule = 0;
	auto textureToBoundIterator = _textures.begin();
	for (; currentModule < numModules && textureToBoundIterator != _textures.end(); ++currentModule)
	{
		if ((*textureToBoundIterator)->GetCurrentModule() < 0)
		{
			if (!moduleLock[currentModule])
			{
				auto& texture = m_bindedTextures[currentModule];
				if (texture)
					texture->Unbind();

				texture = *textureToBoundIterator;
				texture->Bind(currentModule);
				textureToBoundIterator++;
			}
		}
		else
			textureToBoundIterator++;
	}
}

void StateManager::BindFramebuffer(const std::shared_ptr<IFrameBuffer>& _framebuffer)
{
	assert(_framebuffer);

	if (m_activeFramebuffer && m_activeFramebuffer == _framebuffer)
		return;

	_framebuffer->Bind();
	m_activeFramebuffer = _framebuffer;
}

void StateManager::BindShader(const std::shared_ptr<IShaderProgram>& _shader)
{
	assert(_shader);

	if (m_activeShader && m_activeShader == _shader)
		return;

	_shader->Bind();
	m_activeShader = _shader;
}

void StateManager::BindVertexArray(const std::shared_ptr<IVertexArray>& _vertexArray)
{
	assert(_vertexArray);

	if (m_activeVertexArray && m_activeVertexArray == _vertexArray)
		return;

	_vertexArray->Bind();
	m_activeVertexArray = _vertexArray;
}