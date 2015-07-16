#include "GlVertexBuffer.h"

#include <map>
#include <unordered_map>

using namespace AT2;

template<typename T> struct buffer_type_traits;

#define DECLARE_BUFFER_TYPE(T, DataType, Cnt) template <> struct buffer_type_traits<T> { \
	static const BufferTypeInfo typeInfo; \
	}; \
	const BufferTypeInfo buffer_type_traits<T>::typeInfo = {/*#T,*/ DataType, Cnt, sizeof(T), false}; \
	template class GlVertexBuffer<T>;


DECLARE_BUFFER_TYPE(char, BufferDataType::Byte, 1);
DECLARE_BUFFER_TYPE(unsigned char, BufferDataType::UByte, 1);
DECLARE_BUFFER_TYPE(short, BufferDataType::Short, 1);
DECLARE_BUFFER_TYPE(unsigned short, BufferDataType::UShort, 1);
DECLARE_BUFFER_TYPE(int, BufferDataType::Int, 1);
DECLARE_BUFFER_TYPE(unsigned int, BufferDataType::UInt, 1);
DECLARE_BUFFER_TYPE(float, BufferDataType::Float, 1);
DECLARE_BUFFER_TYPE(double, BufferDataType::Double, 1);

DECLARE_BUFFER_TYPE(glm::bvec2, BufferDataType::Byte, 2);
DECLARE_BUFFER_TYPE(glm::bvec3, BufferDataType::Byte, 3);
DECLARE_BUFFER_TYPE(glm::bvec4, BufferDataType::Byte, 4);

DECLARE_BUFFER_TYPE(glm::ivec2, BufferDataType::Int, 2);
DECLARE_BUFFER_TYPE(glm::ivec3, BufferDataType::Int, 3);
DECLARE_BUFFER_TYPE(glm::ivec4, BufferDataType::Int, 4);

DECLARE_BUFFER_TYPE(glm::vec2, BufferDataType::Float, 2);
DECLARE_BUFFER_TYPE(glm::vec3, BufferDataType::Float, 3);
DECLARE_BUFFER_TYPE(glm::vec4, BufferDataType::Float, 4);

DECLARE_BUFFER_TYPE(glm::dvec2, BufferDataType::Double, 2);
DECLARE_BUFFER_TYPE(glm::dvec3, BufferDataType::Double, 3);
DECLARE_BUFFER_TYPE(glm::dvec4, BufferDataType::Double, 4);


GlVertexBufferBase::GlVertexBufferBase(VertexBufferType bufferType) :
	m_publicType(bufferType),
	m_id(0)
{
	m_privateType = DetermineGlBufferType(bufferType);

	glGenBuffers(1, &m_id);
}

//TODO: make optimized?
GlVertexBufferBase::GlBufferType GlVertexBufferBase::DetermineGlBufferType(VertexBufferType bufferType) const
{
	static const std::unordered_map<VertexBufferType, GlVertexBufferBase::GlBufferType> map = {
		std::pair<VertexBufferType, GlVertexBufferBase::GlBufferType>(VertexBufferType::ArrayBuffer, GlVertexBufferBase::GlBufferType::ArrayBuffer),
		std::pair<VertexBufferType, GlVertexBufferBase::GlBufferType>(VertexBufferType::IndexBuffer, GlVertexBufferBase::GlBufferType::ElementArrayBuffer),
		std::pair<VertexBufferType, GlVertexBufferBase::GlBufferType>(VertexBufferType::UniformBuffer, GlVertexBufferBase::GlBufferType::UniformBuffer)
	};

	auto iterator = map.find(bufferType);
	assert(iterator != map.end());

	return iterator->second;
}

GlVertexBufferBase::GlBufferDataType GlVertexBufferBase::DetermineGlDataType(const BufferDataType& commonType) const
{
	static const std::unordered_map<BufferDataType, GlVertexBufferBase::GlBufferDataType> map = {
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::Byte, GlVertexBufferBase::GlBufferDataType::Byte),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::UByte, GlVertexBufferBase::GlBufferDataType::UByte),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::Short, GlVertexBufferBase::GlBufferDataType::Short),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::UShort, GlVertexBufferBase::GlBufferDataType::UShort),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::Int, GlVertexBufferBase::GlBufferDataType::Int),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::UInt, GlVertexBufferBase::GlBufferDataType::UInt),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::HalfFloat, GlVertexBufferBase::GlBufferDataType::HalfFloat),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::Float, GlVertexBufferBase::GlBufferDataType::Float),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::Double, GlVertexBufferBase::GlBufferDataType::Double),
		std::pair<BufferDataType, GlVertexBufferBase::GlBufferDataType>(BufferDataType::Fixed, GlVertexBufferBase::GlBufferDataType::Fixed)
	};

	auto iterator = map.find(commonType);
	assert(iterator != map.end());

	return iterator->second;
}

void AT2::GlVertexBufferBase::SetDataType(const BufferTypeInfo & typeInfo)
{
	memcpy(&m_typeInfo, &typeInfo, sizeof(BufferTypeInfo));
	m_typeInfo.GlDataType = DetermineGlDataType(typeInfo.Type);
}

template <typename T>
GlVertexBuffer<T>::GlVertexBuffer(VertexBufferType _bufferType, GLsizeiptr _size, const T* _data) :
	GlVertexBufferBase(_bufferType),
	m_mappedData(nullptr)
{
	SetDataType(buffer_type_traits<T>::typeInfo);
	m_length = _size;

	const GLsizeiptr dataLength = sizeof(T) * m_length;
	glNamedBufferDataEXT(m_id, dataLength, _data, static_cast<GLenum>(m_usageHint));
}
/*
template<typename T>
GlVertexBuffer<T>::GlVertexBuffer(GlBufferType _type, GLsizeiptr _size) :
	GlVertexBuffer<T>::GlVertexBuffer(_type, _size, std::unique_ptr<T[]>(new T[_size]).get())
{
}
*/

template <typename T>
GlVertexBuffer<T>::~GlVertexBuffer()
{
	if (m_mappedData)
		Unlock();

	glDeleteBuffers(1, &m_id);
}

template <typename T>
void GlVertexBuffer<T>::Bind()
{
	glBindBuffer(static_cast<GLenum>(m_privateType), m_id);

}

template <typename T>
void GlVertexBuffer<T>::SetData(unsigned int _size, const T* _data)
{
	glNamedBufferDataEXT(m_id, _size, _data, static_cast<GLenum>(m_usageHint));
}

template <typename T>
Utils::wraparray<T> GlVertexBuffer<T>::Lock()
{
	if (!m_mappedData)
		m_mappedData = reinterpret_cast<T*>(glMapNamedBufferEXT(m_id, GL_READ_WRITE));
	else
		Log::Warning() << "VertexBuffer: redundant Lock() call" << std::endl;
	//glMapNamedBufferRangeEXT(m_id, 0, _size, );
	
	return Utils::wraparray<T>(m_length, m_mappedData);
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