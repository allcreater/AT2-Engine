#ifndef AT2_GL_VERTEXARRAY_H
#define AT2_GL_VERTEXARRAY_H

#include "glVertexBuffer.h"

namespace AT2
{

class GlVertexArray : public IVertexArray
{
public:
	GlVertexArray(IRendererCapabilities* rendererCapabilities);
	~GlVertexArray();

public:
	void Bind() override;
	unsigned int GetId() const override { return m_id; }

	void SetIndexBuffer(const std::shared_ptr<IVertexBuffer>& buffer) override;
	std::shared_ptr<IVertexBuffer> GetIndexBuffer() const override;
	//virtual std::shared_ptr<GlVertexBufferBase> GetOrSetIndexBuffer() const;

	void SetVertexBuffer(unsigned int index, const std::shared_ptr<IVertexBuffer>& buffer) override;
	std::shared_ptr<IVertexBuffer> GetVertexBuffer(unsigned int index) const override;
	
private:
	GLuint m_id;

	//we are sure that all buffer will be at least GlVertexBufferBase or derived types
	Utils::dynarray<std::shared_ptr<IVertexBuffer>> m_buffers;
	std::shared_ptr<IVertexBuffer> m_indexBuffer;
};

}
#endif