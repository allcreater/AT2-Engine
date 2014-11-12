#include "GlVertexArray.h"
#include <map>

using namespace AT2;

template<typename T>
struct GlTypeTraits;

#define REGISTER_GL_TYPE(T, GlType, GlVecLength) template <> struct GlTypeTraits<T> { \
    static const char* name; \
	static const GlVertexBufferBase::GlBufferDataType gl_type; \
	static const GlVertexBufferBase::GlBufferDataVectorLength gl_vectorLength; \
	}; \
	const char* GlTypeTraits<T>::name = #T; \
	const GlVertexBufferBase::GlBufferDataType GlTypeTraits<T>::gl_type = GlType; \
	const GlVertexBufferBase::GlBufferDataVectorLength GlTypeTraits<T>::gl_vectorLength = GlVecLength; \
	template class GlVertexBuffer<T>

REGISTER_GL_TYPE(GLbyte,	GlVertexBufferBase::GlBufferDataType::Byte,		GlVertexBufferBase::GlBufferDataVectorLength::One);
REGISTER_GL_TYPE(GLubyte,	GlVertexBufferBase::GlBufferDataType::UByte,	GlVertexBufferBase::GlBufferDataVectorLength::One);
REGISTER_GL_TYPE(GLshort,	GlVertexBufferBase::GlBufferDataType::Short,	GlVertexBufferBase::GlBufferDataVectorLength::One);
REGISTER_GL_TYPE(GLushort,	GlVertexBufferBase::GlBufferDataType::UShort,	GlVertexBufferBase::GlBufferDataVectorLength::One);
REGISTER_GL_TYPE(GLint,		GlVertexBufferBase::GlBufferDataType::Int,		GlVertexBufferBase::GlBufferDataVectorLength::One);
REGISTER_GL_TYPE(GLuint,	GlVertexBufferBase::GlBufferDataType::UInt,		GlVertexBufferBase::GlBufferDataVectorLength::One);
//REGISTER_GL_TYPE(GLfixed,	GlVertexBufferBase::GlBufferDataType::Fixed,	GlVertexBufferBase::GlBufferDataVectorLength::One);
REGISTER_GL_TYPE(GLfloat,	GlVertexBufferBase::GlBufferDataType::Float,	GlVertexBufferBase::GlBufferDataVectorLength::One);
REGISTER_GL_TYPE(GLdouble,	GlVertexBufferBase::GlBufferDataType::Double,	GlVertexBufferBase::GlBufferDataVectorLength::One);

REGISTER_GL_TYPE(glm::bvec2,	GlVertexBufferBase::GlBufferDataType::Byte,		GlVertexBufferBase::GlBufferDataVectorLength::Two);
REGISTER_GL_TYPE(glm::bvec3,	GlVertexBufferBase::GlBufferDataType::Byte,		GlVertexBufferBase::GlBufferDataVectorLength::Three);
REGISTER_GL_TYPE(glm::bvec4,	GlVertexBufferBase::GlBufferDataType::Byte,		GlVertexBufferBase::GlBufferDataVectorLength::Four);
REGISTER_GL_TYPE(glm::ivec2,	GlVertexBufferBase::GlBufferDataType::Int,		GlVertexBufferBase::GlBufferDataVectorLength::Two);
REGISTER_GL_TYPE(glm::ivec3,	GlVertexBufferBase::GlBufferDataType::Int,		GlVertexBufferBase::GlBufferDataVectorLength::Three);
REGISTER_GL_TYPE(glm::ivec4,	GlVertexBufferBase::GlBufferDataType::Int,		GlVertexBufferBase::GlBufferDataVectorLength::Four);
REGISTER_GL_TYPE(glm::vec2,		GlVertexBufferBase::GlBufferDataType::Float,	GlVertexBufferBase::GlBufferDataVectorLength::Two);
REGISTER_GL_TYPE(glm::vec3,		GlVertexBufferBase::GlBufferDataType::Float,	GlVertexBufferBase::GlBufferDataVectorLength::Three);
REGISTER_GL_TYPE(glm::vec4,		GlVertexBufferBase::GlBufferDataType::Float,	GlVertexBufferBase::GlBufferDataVectorLength::Four);
REGISTER_GL_TYPE(glm::dvec2,	GlVertexBufferBase::GlBufferDataType::Double,	GlVertexBufferBase::GlBufferDataVectorLength::Two);
REGISTER_GL_TYPE(glm::dvec3,	GlVertexBufferBase::GlBufferDataType::Double,	GlVertexBufferBase::GlBufferDataVectorLength::Three);
REGISTER_GL_TYPE(glm::dvec4,	GlVertexBufferBase::GlBufferDataType::Double,	GlVertexBufferBase::GlBufferDataVectorLength::Four);

template <typename T>
GlVertexBuffer<T>::GlVertexBuffer(BufferType _type, GLsizeiptr _size, const T* _data) : m_type(_type), m_mappedData(nullptr)
{
	UsageHint = GlVertexBufferBase::BufferUsageHint::StaticDraw;

	ElementTypeInfo.IsNormalized = false;
	ElementTypeInfo.DataType = GlTypeTraits<T>::gl_type;
	ElementTypeInfo.VectorLength = GlTypeTraits<T>::gl_vectorLength;
	ElementTypeInfo.Stride = sizeof(T);
	Log::Info() << "Vertex array type is " << GlTypeTraits<T>::name << std::endl;

	const GLsizeiptr dataLength = sizeof(T) * _size;

	glGenBuffers(1, &m_id);
	glNamedBufferDataEXT(m_id, dataLength, _data, static_cast<GLenum>(UsageHint));
}

template <typename T>
GlVertexBuffer<T>::~GlVertexBuffer()
{
	glDeleteBuffers(1, &m_id);
}

template <typename T>
void GlVertexBuffer<T>::Bind()
{
	glBindBuffer(static_cast<GLenum>(m_type), m_id);

}

template <typename T>
void GlVertexBuffer<T>::SetData(unsigned int _size, const T* _data)
{
	glNamedBufferDataEXT(m_id, _size, _data, static_cast<GLenum>(UsageHint));
}

template <typename T>
T* GlVertexBuffer<T>::Lock()
{
	if (!m_mappedData)
		m_mappedData = reinterpret_cast<T*>(glMapNamedBufferEXT(m_id, static_cast<GLenum>(UsageHint)));
	else
		Log::Warning() << "VertexBuffer: redundant Lock() call" << std::endl;

	return m_mappedData;
}

template <typename T>
void GlVertexBuffer<T>::Unlock()
{
	if (m_mappedData)
	{
		glUnmapNamedBufferEXT(m_id);
		m_mappedData = nullptr;
	}
	else
		Log::Warning() << "VertexBuffer: redundant Unlock() call" << std::endl;
}

GlVertexArray::GlVertexArray()
{
	glGenVertexArrays(1, &m_id);

	GLint maxAttribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
	m_buffers.resize(maxAttribs);
}

void GlVertexArray::Bind()
{
	glBindVertexArray(m_id);
	if (m_indexBuffer)
		m_indexBuffer->Bind();
}

GlVertexArray::~GlVertexArray()
{
	glDeleteVertexArrays(1, &m_id);
}

void GlVertexArray::SetIndexBuffer(const std::shared_ptr<GlVertexBufferBase>& buffer)
{
	if (buffer->GetType() != GlVertexBufferBase::BufferType::ElementArrayBuffer)
		throw AT2Exception("GlVertexBuffer: trying to set not index buffer as index buffer");

	m_indexBuffer = buffer;
}

std::shared_ptr<GlVertexBufferBase> GlVertexArray::GetIndexBuffer() const
{
	return m_indexBuffer;
}

void GlVertexArray::SetVertexBuffer(unsigned int index, const std::shared_ptr<GlVertexBufferBase>& buffer)
{
	if (buffer->GetType() == GlVertexBufferBase::BufferType::ElementArrayBuffer)
		throw AT2Exception("GlVertexBuffer: trying to set index buffer as attribute buffer");

	m_buffers.at(index) = buffer;

	if (buffer)
	{
		glVertexArrayVertexAttribOffsetEXT(m_id, buffer->GetId(), index, static_cast<GLint>(buffer->ElementTypeInfo.VectorLength), static_cast<GLenum>(buffer->ElementTypeInfo.DataType), buffer->ElementTypeInfo.IsNormalized ? GL_TRUE : GL_FALSE, buffer->ElementTypeInfo.Stride, 0);
		glEnableVertexArrayAttribEXT(m_id, index);
	}
	else
	{
		glDisableVertexArrayAttribEXT(m_id, index);
	}
}
std::shared_ptr<GlVertexBufferBase> GlVertexArray::GetVertexBuffer(unsigned int index) const
{
	return m_buffers.at(index);
}
/*
template class AT2::GlVertexBuffer<glm::vec3>;
template class AT2::GlVertexBuffer<GLuint>;
*/