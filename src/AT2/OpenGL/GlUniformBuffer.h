#ifndef AT2_GL_UNIFORMBUFFER_H
#define AT2_GL_UNIFORMBUFFER_H

#include "GlVertexBuffer.h"
#include "GlShaderProgram.h"

namespace AT2
{

class GlUniformBuffer : public GlVertexBuffer, public IUniformContainer
{
public:
	GlUniformBuffer(std::shared_ptr<GlShaderProgram::UniformBufferInfo> ubi);
	~GlUniformBuffer();

public:
	//doubles
	void SetUniform(const str& name, const double& value)		override;
	void SetUniform(const str& name, const glm::dvec2& value)	override;
	void SetUniform(const str& name, const glm::dvec3& value)	override;
	void SetUniform(const str& name, const glm::dvec4& value)	override;

	void SetUniform(const str& name, const glm::dmat2& value)	override;
	void SetUniform(const str& name, const glm::dmat3& value)	override;
	void SetUniform(const str& name, const glm::dmat4& value)	override;

	//floats
	void SetUniform(const str& name, const float& value)		override;
	void SetUniform(const str& name, const glm::vec2& value)	override;
	void SetUniform(const str& name, const glm::vec3& value)	override;
	void SetUniform(const str& name, const glm::vec4& value)	override;

	void SetUniform(const str& name, const glm::mat2& value)	override;
	void SetUniform(const str& name, const glm::mat3& value)	override;
	void SetUniform(const str& name, const glm::mat4& value)	override;

	//integers
	void SetUniform(const str& name, const int& value)			override;
	void SetUniform(const str& name, const glm::ivec2& value)	override;
	void SetUniform(const str& name, const glm::ivec3& value)	override;
	void SetUniform(const str& name, const glm::ivec4& value)	override;

	//unsigned integers
	void SetUniform(const str& name, const glm::uint& value)	override;
	void SetUniform(const str& name, const glm::uvec2& value)	override;
	void SetUniform(const str& name, const glm::uvec3& value)	override;
	void SetUniform(const str& name, const glm::uvec4& value)	override;

	//textures
	void SetUniform(const str& name, std::weak_ptr<const ITexture> value) override;

	virtual void Bind() override;
	void SetBindingPoint(unsigned int index) { m_bindingPoint = index; }

private:
	template <typename T> void SetUniformInternal(const str& name, const T& value, size_t size = sizeof(T));

private:
	std::shared_ptr<GlShaderProgram::UniformBufferInfo> m_ubi;
	GLuint m_bindingPoint;
};

}

#endif