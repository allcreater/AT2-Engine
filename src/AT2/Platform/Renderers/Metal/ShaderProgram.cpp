#include "ShaderProgram.h"

using namespace AT2::Metal;

ShaderProgram::ShaderProgram()
{
	
}

ShaderProgram::~ShaderProgram()
{
	
}

void ShaderProgram::Bind()
{
	
}

bool ShaderProgram::IsActive() const noexcept
{
    return false;
}

std::unique_ptr<IUniformContainer> ShaderProgram::CreateAssociatedUniformStorage(std::string_view blockName)
{
    return std::unique_ptr<IUniformContainer>();
}

void ShaderProgram::AttachShader(std::string_view data, ShaderType type)
{
	
}

void ShaderProgram::SetUBO(std::string_view blockName, unsigned int index)
{
	
}

void ShaderProgram::SetUniform(std::string_view name, Uniform value)
{
	
}

void ShaderProgram::SetUniformArray(std::string_view name, UniformArray value)
{
	
}

bool ShaderProgram::TryCompile()
{
    return false;
}

void ShaderProgram::CleanUp() {}
