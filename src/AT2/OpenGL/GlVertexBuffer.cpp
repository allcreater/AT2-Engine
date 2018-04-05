#include "GlVertexBuffer.h"

#include <map>
#include <unordered_map>
#include <cassert>

using namespace AT2;

GlVertexBuffer::GlVertexBuffer(VertexBufferType bufferType) :
	m_publicType(bufferType),
	m_id(0)
{
	m_privateType = DetermineGlBufferType(bufferType);

	glGenBuffers(1, &m_id);
}

//TODO: make optimized?
GlVertexBuffer::GlBufferType GlVertexBuffer::DetermineGlBufferType(VertexBufferType bufferType) const
{
	static const std::unordered_map<VertexBufferType, GlVertexBuffer::GlBufferType> map = {
		std::pair<VertexBufferType, GlVertexBuffer::GlBufferType>(VertexBufferType::ArrayBuffer, GlVertexBuffer::GlBufferType::ArrayBuffer),
		std::pair<VertexBufferType, GlVertexBuffer::GlBufferType>(VertexBufferType::IndexBuffer, GlVertexBuffer::GlBufferType::ElementArrayBuffer),
		std::pair<VertexBufferType, GlVertexBuffer::GlBufferType>(VertexBufferType::UniformBuffer, GlVertexBuffer::GlBufferType::UniformBuffer)
	};

	auto iterator = map.find(bufferType);
	assert(iterator != map.end());

	return iterator->second;
}

GlVertexBuffer::GlBufferDataType GlVertexBuffer::DetermineGlDataType(const BufferDataType& commonType) const
{
	static const std::unordered_map<BufferDataType, GlVertexBuffer::GlBufferDataType> map = {
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::Byte, GlVertexBuffer::GlBufferDataType::Byte),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::UByte, GlVertexBuffer::GlBufferDataType::UByte),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::Short, GlVertexBuffer::GlBufferDataType::Short),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::UShort, GlVertexBuffer::GlBufferDataType::UShort),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::Int, GlVertexBuffer::GlBufferDataType::Int),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::UInt, GlVertexBuffer::GlBufferDataType::UInt),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::HalfFloat, GlVertexBuffer::GlBufferDataType::HalfFloat),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::Float, GlVertexBuffer::GlBufferDataType::Float),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::Double, GlVertexBuffer::GlBufferDataType::Double),
		std::pair<BufferDataType, GlVertexBuffer::GlBufferDataType>(BufferDataType::Fixed, GlVertexBuffer::GlBufferDataType::Fixed)
	};

	auto iterator = map.find(commonType);
	assert(iterator != map.end());

	return iterator->second;
}

void AT2::GlVertexBuffer::SetDataType(const BufferTypeInfo & typeInfo)
{
	memcpy(&m_typeInfo, &typeInfo, sizeof(BufferTypeInfo));
	m_typeInfo.GlDataType = DetermineGlDataType(typeInfo.Type);
}

void GlVertexBuffer::Bind()
{
	glBindBuffer(static_cast<GLenum>(m_privateType), m_id);
}

GlVertexBuffer::~GlVertexBuffer()
{
	glDeleteBuffers(1, &m_id);
}

void GlVertexBuffer::SetData(unsigned int _size, const void* _data)
{
	assert(GLEW_EXT_direct_state_access && __glewNamedBufferDataEXT);
	glNamedBufferDataEXT(m_id, _size, _data, static_cast<GLenum>(m_usageHint));
}
