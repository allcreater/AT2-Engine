#ifndef AT2_GL_UNIFORMCONTAINER_H
#define AT2_GL_UNIFORMCONTAINER_H

#include "GlShaderProgram.h"
#include "GlTexture.h"

namespace AT2
{

class IGlUniform
{
	friend class GlUniformContainer;

public:
	IGlUniform() : m_location(-1) {}
	virtual ~IGlUniform() {};

public:
	//doubles
	virtual void set(const GLdouble& value)		{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::dvec2& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::dvec3& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::dvec4& value)	{ throw std::logic_error("wrong uniform type"); }

	virtual void set(const glm::dmat2& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::dmat3& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::dmat4& value)	{ throw std::logic_error("wrong uniform type"); }

	//floats
	virtual void set(const GLfloat& value)		{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::vec2& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::vec3& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::vec4& value)	{ throw std::logic_error("wrong uniform type"); }

	virtual void set(const glm::mat2& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::mat3& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::mat4& value)	{ throw std::logic_error("wrong uniform type"); }

	//integers
	virtual void set(const GLint& value)		{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::ivec2& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::ivec3& value)	{ throw std::logic_error("wrong uniform type"); }
	virtual void set(const glm::ivec4& value)	{ throw std::logic_error("wrong uniform type"); }

	//texture
	virtual void set(std::weak_ptr<const ITexture> value) { throw std::logic_error("wrong uniform type"); }

	virtual void Bind() = 0;
	virtual void Bind(GLuint programID) = 0;

	GLint GetLocation() { return m_location; }

private:
	GLint m_location;
};

class GlUniformContainer : public IUniformContainer
{
public:
	GlUniformContainer(std::weak_ptr<GlShaderProgram> program);
	~GlUniformContainer();

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
	void SetUniform(const str& name, const int& value)		override;
	void SetUniform(const str& name, const glm::ivec2& value)	override;
	void SetUniform(const str& name, const glm::ivec3& value)	override;
	void SetUniform(const str& name, const glm::ivec4& value)	override;

	//textures
	void SetUniform(const str& name, std::weak_ptr<const ITexture> value) override;

	virtual void Bind() override;

private:
	template <typename T>
	IGlUniform* GetOrCreateUniform(const str& name);

private:
	std::map<str, IGlUniform*> m_uniformsMap; //TODO achtung: поле name и map key дублируются
	std::weak_ptr<GlShaderProgram> m_program;
};

}

#endif