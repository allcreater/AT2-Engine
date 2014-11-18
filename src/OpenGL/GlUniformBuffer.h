#ifndef AT2_GL_UNIFORMBUFFER_H
#define AT2_GL_UNIFORMBUFFER_H

#include "GlVertexBuffer.h"
#include "GlShaderProgram.h"

namespace AT2
{

class GlUniformBuffer : public GlVertexBufferBase
{
public:
	GlUniformBuffer(std::shared_ptr<GlShaderProgram::UniformBufferInfo> ubi);
	~GlUniformBuffer();

public:
	virtual GlBufferType GetType() const { return GlBufferType::UniformBuffer; }

	virtual void Bind();

	template <typename T>
	void SetUniform(const str& name, const T& value)
	{
		const GlShaderProgram::UniformInfo* ui = m_ubi->GetUniformInfo(name);
		if (!ui)
			return;

		auto data = reinterpret_cast<GLbyte*>(glMapNamedBufferEXT(m_id, GL_WRITE_ONLY));
		memcpy(data + ui->Offset, glm::value_ptr(value), sizeof(T));
		glUnmapNamedBufferEXT(m_id);
	}

	void SetBindingPoint(unsigned int index) { m_bindingPoint = index; }

private:
	std::shared_ptr<GlShaderProgram::UniformBufferInfo> m_ubi;
	GLuint m_bindingPoint;
};

}

#endif