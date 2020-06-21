#include "GlUniformContainer.h"

#include <utility>

using namespace AT2;
using namespace glm;


GlUniformContainer::GlUniformContainer(std::weak_ptr<GlShaderProgram> _program)
{
	SetProgram(std::move(_program));
}

void GlUniformContainer::SetUniform(const str &name, const Uniform &value)
{
	m_uniformsMap[name] = value;
}

void GlUniformContainer::SetUniform(const str &name, std::weak_ptr<const ITexture> value)
{
	m_texturesMap[name] = std::move(value);
}

//TODO: maybe it's better to detach uniform container from specific program at all?
void GlUniformContainer::Bind()
{
	auto program = m_program.lock();
	if (!program)
		throw AT2::AT2Exception(AT2Exception::ErrorCase::Shader, "GlUniformContainer: try to bind uniform container when parent program is missing");


	for (const auto& [name, value] : m_uniformsMap)
		program->SetUniform(name, value);

	for (const auto& [name, value] : m_texturesMap)
		program->SetUniform(name, value.lock()->GetCurrentModule());
}