#include "UniformContainer.h"

#include <utility>

using namespace AT2;
using namespace glm;


void UniformContainer::SetUniform(std::string_view name, const Uniform& value)
{
    const auto& [it, isInserted] = m_uniformsMap.emplace(name, value);
    if (!isInserted)
        it->second = value;
}

void UniformContainer::SetUniform(std::string_view name, const std::shared_ptr<ITexture>& value)
{
    const auto& [it, isInserted] = m_texturesMap.emplace(name, value);
    if (!isInserted)
        it->second = value;
}

void UniformContainer::Bind(IStateManager& stateManager) const
{
    const auto& program = stateManager.GetActiveShader();
    if (!program)
        throw AT2ShaderException("GlUniformContainer: try to bind uniform container when parent program is missing");

    for (const auto& [name, value] : m_uniformsMap)
        program->SetUniform(name, value);

    TextureSet set; //TODO: just a crutch, make it correct!!!
    for (const auto& [name, value] : m_texturesMap)
        set.insert(value);
    stateManager.BindTextures(set);

    for (const auto& [name, value] : m_texturesMap)
        program->SetUniform(name, value->GetCurrentModule());
}
