#ifndef AT2_GL_VERTEXBUFFER_H
#define AT2_GL_VERTEXBUFFER_H

#include "AT2lowlevel.h"

namespace AT2
{
	
class GlVertexBuffer : public IVertexBuffer
{
public:
	GlVertexBuffer(VertexBufferType bufferType);
	~GlVertexBuffer() override;

public:
	unsigned int GetId() const override					{ return m_id; }
	VertexBufferType GetType() const override			{ return m_publicType; }

	const BufferTypeInfo& GetDataType() const override	{ return m_typeInfo; }
	void				SetDataType(const BufferTypeInfo& typeInfo) override;

	size_t GetLength() const override					{ return m_length; }

	void Bind() override;
	void SetData(unsigned int length, const void* data) override;
protected:
	enum class GlBufferType : GLenum//TODO: Not full list of types!
	{
		ArrayBuffer = GL_ARRAY_BUFFER,
		ElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,
		UniformBuffer = GL_UNIFORM_BUFFER
	};

	enum class GlBufferUsageHint : GLenum
	{
		StreamDraw = GL_STREAM_DRAW,
		StreamRead = GL_STREAM_READ,
		StreamCopy = GL_STREAM_COPY,
		StaticDraw = GL_STATIC_DRAW,
		StaticRead = GL_STATIC_READ,
		StaticCopy = GL_STATIC_COPY,
		DynamicDraw = GL_DYNAMIC_DRAW,
		DynamicRead = GL_DYNAMIC_READ,
		DynamicCopy = GL_DYNAMIC_COPY
	};

	enum class GlBufferDataType : GLenum
	{
		//both for glVertexAttribIPointer and glVertexAttribPointer
		Byte = GL_BYTE,
		UByte = GL_UNSIGNED_BYTE,
		Short = GL_SHORT,
		UShort = GL_UNSIGNED_SHORT,
		Int = GL_INT,
		UInt = GL_UNSIGNED_INT,
		//for glVertexAttribPointer only
		HalfFloat = GL_HALF_FLOAT,
		Float = GL_FLOAT,
		Double = GL_DOUBLE,
		Fixed = GL_FIXED,
		//???
		Int2_10_10_10 = GL_INT_2_10_10_10_REV,
		UInt2_10_10_10 = GL_UNSIGNED_INT_2_10_10_10_REV,
		UInt10F_11F_11F = GL_UNSIGNED_INT_10F_11F_11F_REV
	};

public:
	struct GlBufferTypeInfo : public BufferTypeInfo
	{
		//BufferTypeInfo();

		GlBufferDataType GlDataType;
	};

protected:
	GlBufferType		DetermineGlBufferType (VertexBufferType bufferType) const;
	GlBufferDataType	DetermineGlDataType (const BufferDataType& dataType) const;

protected:
	GLuint m_id;
	size_t m_length;

	VertexBufferType m_publicType;
	GlBufferType m_privateType;
	GlBufferUsageHint m_usageHint = GlBufferUsageHint::StaticDraw;
	GlBufferTypeInfo m_typeInfo;
};

}

#endif