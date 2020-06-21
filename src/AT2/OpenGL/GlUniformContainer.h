#ifndef AT2_GL_UNIFORMCONTAINER_H
#define AT2_GL_UNIFORMCONTAINER_H

#include <utility>


#include "GlShaderProgram.h"
#include "GlTexture.h"

namespace AT2
{

class GlUniformContainer : public IUniformContainer
{
public:
	GlUniformContainer(std::weak_ptr<GlShaderProgram> program);

public:
	void SetUniform(const str& name, const Uniform& value) override;
	//textures
	void SetUniform(const str& name, std::weak_ptr<const ITexture> value) override;

	void Bind() override;
	
	void SetProgram(std::weak_ptr<GlShaderProgram> program) { m_program = std::move(program); }
	std::weak_ptr<GlShaderProgram> GetProgram() { return m_program; }

private:
	std::unordered_map<str, Uniform> m_uniformsMap;
	std::unordered_map<str, std::weak_ptr<const ITexture>> m_texturesMap;
	std::weak_ptr<GlShaderProgram> m_program;
};

}

#endif