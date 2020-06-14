#ifndef AT2_GL_DRAWPRIMITIVE_H
#define AT2_GL_DRAWPRIMITIVE_H

#include "AT2lowlevel.h"

namespace AT2
{

enum class GlDrawPrimitiveType
{
	Points = GL_POINTS,
	LineStrip = GL_LINE_STRIP,
	LineLoop = GL_LINE_LOOP,
	Lines = GL_LINES,
	LineStripAdjacency = GL_LINE_STRIP_ADJACENCY,
	LinesAdjacency = GL_LINES_ADJACENCY,
	TriangleStrip = GL_TRIANGLE_STRIP,
	TriangleFan = GL_TRIANGLE_FAN,
	Triangles = GL_TRIANGLES,
	TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
	TrianglesAdjacency = GL_TRIANGLES_ADJACENCY,
	Patches = GL_PATCHES
};

class GlDrawPrimitive : public IDrawPrimitive
{
public:
	GlDrawPrimitive(GlDrawPrimitiveType mode) :
        m_primitiveMode(mode)
    {}

	GlDrawPrimitiveType GetPrimitiveMode() const {return m_primitiveMode;}

private:
	GlDrawPrimitiveType m_primitiveMode;
};


class GlDrawArraysPrimitive : public GlDrawPrimitive
{
public:
	GlDrawArraysPrimitive(GlDrawPrimitiveType mode, GLint first, GLsizei count) :
        GlDrawPrimitive(mode),
        m_firstIndex(first),
        m_count(count)
    {}

public:
	void Draw() const override
	{
		glDrawArrays(static_cast<GLenum>(GetPrimitiveMode()), m_firstIndex, m_count);
	}

private:
	GLint m_firstIndex;
	GLsizei m_count;
	
};

class GlDrawElementsPrimitive : public GlDrawPrimitive
{
public:
	enum class IndicesType
	{
		UnsignedByte = GL_UNSIGNED_BYTE,
		UnsignedShort = GL_UNSIGNED_SHORT,
		UnsignedInt = GL_UNSIGNED_INT 
	};

public:
	GlDrawElementsPrimitive(GlDrawPrimitiveType mode, GLsizei count, IndicesType indicesType, const void* first) :
        GlDrawPrimitive(mode),
        m_count(count),
        m_indicesType(indicesType),
        m_firstIndex(first)
    {}

public:
	void Draw() const override
	{
		glDrawElements(static_cast<GLenum>(GetPrimitiveMode()), m_count, static_cast<GLenum>(m_indicesType), m_firstIndex);
	}

private:
	GLsizei m_count;
	IndicesType m_indicesType;
	const void* m_firstIndex;
};

class GlDrawElementsInstancedPrimitive : public GlDrawPrimitive
{
public:
    enum class IndicesType
    {
        UnsignedByte = GL_UNSIGNED_BYTE,
        UnsignedShort = GL_UNSIGNED_SHORT,
        UnsignedInt = GL_UNSIGNED_INT
    };

public:
    GlDrawElementsInstancedPrimitive(GlDrawPrimitiveType mode, GLsizei count, IndicesType indicesType, const void* first, GLsizei numInstances) :
        GlDrawPrimitive(mode),
        m_count(count),
        m_indicesType(indicesType),
        m_firstIndex(first),
        m_numInstances(numInstances)
    {}

public:
    void Draw() const override
    {
        glDrawElementsInstanced(static_cast<GLenum>(GetPrimitiveMode()), m_count, static_cast<GLenum>(m_indicesType), m_firstIndex, m_numInstances);
    }

private:
    GLsizei m_count;
    IndicesType m_indicesType;
    const void* m_firstIndex;
    GLsizei m_numInstances;
};

}

#endif