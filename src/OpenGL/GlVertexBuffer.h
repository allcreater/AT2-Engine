#ifndef AT2_GL_VERTEXBUFFER_H
#define AT2_GL_VERTEXBUFFER_H

#include "AT2lowlevel.h"

namespace AT2
{
	
class GlVertexBufferBase : public IVertexBuffer
{
public:
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
	} UsageHint;

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

	enum class GlBufferDataVectorLength
	{
		One = 1,
		Two = 2,
		Three = 3,
		Four = 4
	};

	struct GlBufferTypeInfo
	{
		GlBufferDataType DataType;
		GlBufferDataVectorLength VectorLength; 
		GLsizei Stride;
		bool IsNormalized;
	};

	GlVertexBufferBase() {}

public:
	virtual unsigned int GetId() const { return m_id; }
	virtual GlBufferType GetType() const = 0;
	GlBufferTypeInfo ElementTypeInfo;

protected:
	GLuint m_id;
};


template <typename T>
class GlVertexBuffer : public GlVertexBufferBase, public IBuffer<T>
{
public:
	GlVertexBuffer(GlBufferType type, GLsizeiptr size, const T* data);
	~GlVertexBuffer();

public:
	virtual GlBufferType GetType() const { return m_type; }

	virtual void Bind();

	virtual void SetData(unsigned int length, const T* data);
	virtual T* Lock();
	virtual void Unlock();

private:
	GlBufferType m_type;

	T* m_mappedData;
};


}

#endif