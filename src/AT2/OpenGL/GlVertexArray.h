#ifndef AT2_GL_VERTEXARRAY_H
#define AT2_GL_VERTEXARRAY_H

#include "GlVertexBuffer.h"

namespace AT2
{

class GlVertexArray : public IVertexArray
{
public:
	NON_COPYABLE_OR_MOVABLE(GlVertexArray)

	GlVertexArray(const IRendererCapabilities& rendererCapabilities);
	~GlVertexArray() override;

public:
	void Bind() override;
    [[nodiscard]] unsigned int GetId() const override { return m_id; }

	void SetIndexBuffer(std::shared_ptr<IVertexBuffer> buffer) override;
    [[nodiscard]] std::shared_ptr<IVertexBuffer> GetIndexBuffer() const override;
	//virtual std::shared_ptr<GlVertexBuffer> GetOrSetIndexBuffer() const;

	void SetVertexBuffer(unsigned int index, std::shared_ptr<IVertexBuffer> buffer) override;
	void SetVertexBufferDivisor(unsigned int index, unsigned int divisor = 0) override;
    [[nodiscard]] std::shared_ptr<IVertexBuffer> GetVertexBuffer(unsigned int index) const override;
	
private:
	GLuint m_id;

	//we are sure that all buffer will be at least GlVertexBuffer or derived types
	std::vector<std::shared_ptr<IVertexBuffer>> m_buffers;
	std::shared_ptr<IVertexBuffer> m_indexBuffer;
};

}
#endif
