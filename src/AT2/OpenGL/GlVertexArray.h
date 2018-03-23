#ifndef AT2_GL_VERTEXARRAY_H
#define AT2_GL_VERTEXARRAY_H

#include "glVertexBuffer.h"

namespace AT2
{

class GlVertexArray : public IVertexArray
{
public:
	GlVertexArray(const IRendererCapabilities& rendererCapabilities);
	~GlVertexArray();

public:
	virtual void Bind();
	virtual unsigned int GetId() const { return m_id; }

	virtual void SetIndexBuffer(const std::shared_ptr<GlVertexBufferBase>& buffer);
	virtual std::shared_ptr<GlVertexBufferBase> GetIndexBuffer() const;

	virtual void SetVertexBuffer(unsigned int index, const std::shared_ptr<GlVertexBufferBase>& buffer);
	virtual std::shared_ptr<GlVertexBufferBase> GetVertexBuffer(unsigned int index) const;
	
private:
	GLuint m_id;
	Utils::dynarray<std::shared_ptr<GlVertexBufferBase>> m_buffers;
	std::shared_ptr<GlVertexBufferBase> m_indexBuffer;
};

}
#endif