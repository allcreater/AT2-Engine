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

        void SetIndexBuffer(std::shared_ptr<IVertexBuffer> buffer, BufferDataType type) override;
        [[nodiscard]] std::shared_ptr<IVertexBuffer> GetIndexBuffer() const override { return m_indexBuffer.first; }
        [[nodiscard]] std::optional<BufferDataType> GetIndexBufferType() const override { return m_indexBuffer.second; }

        void SetVertexBuffer(unsigned int index, std::shared_ptr<IVertexBuffer> buffer,
                             BufferTypeInfo binding) override;
        void SetVertexBufferDivisor(unsigned int index, unsigned int divisor = 0) override;
        [[nodiscard]] std::shared_ptr<IVertexBuffer> GetVertexBuffer(unsigned int index) const override;
        [[nodiscard]] std::optional<BufferTypeInfo> GetVertexBufferBinding(unsigned int index) const override;

    private:
        GLuint m_id;

        //we are sure that all buffer will be at least GlVertexBuffer or derived types
        std::vector<std::pair<std::shared_ptr<IVertexBuffer>, BufferTypeInfo>> m_buffers;
        std::pair<std::shared_ptr<IVertexBuffer>, std::optional<BufferDataType>> m_indexBuffer;
    };

} // namespace AT2
#endif
