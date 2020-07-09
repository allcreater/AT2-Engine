#include "UniformContainer.h"

#include <utility>

using namespace AT2;
using namespace glm;


void UniformContainer::SetUniform(const str& name, const Uniform& value)
{
    m_uniformsMap[name] = value;
}

void UniformContainer::SetUniform(const str& name, const std::shared_ptr<ITexture>& value)
{
    m_texturesMap[name] = value;
}

void UniformContainer::Bind(IStateManager& stateManager) const
{
    const auto& program = stateManager.GetActiveShader();
    if (!program)
        throw AT2Exception(AT2Exception::ErrorCase::Shader,
                           "GlUniformContainer: try to bind uniform container when parent program is missing");

    for (const auto& [name, value] : m_uniformsMap)
        program->SetUniform(name, value);

    TextureSet set; //TODO: just a crutch, make it correct!!!
    for (const auto& [name, value] : m_texturesMap)
        set.insert(value);
    stateManager.BindTextures(set);

    for (const auto& [name, value] : m_texturesMap)
        program->SetUniform(name, value->GetCurrentModule());
}
