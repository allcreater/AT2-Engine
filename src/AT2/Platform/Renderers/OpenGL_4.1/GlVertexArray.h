#ifndef AT2_GL_VERTEXARRAY_H
#define AT2_GL_VERTEXARRAY_H

#include "GlBuffer.h"

namespace AT2::OpenGL41
{

    class GlVertexArray : public IVertexArray
    {
    public:
        NON_COPYABLE_OR_MOVABLE(GlVertexArray)

        GlVertexArray(const IRendererCapabilities& rendererCapabilities);
        ~GlVertexArray() override;

    public:
        [[nodiscard]] unsigned int GetId() const noexcept override { return m_id; }

        void SetIndexBuffer(std::shared_ptr<IBuffer> buffer, BufferDataType type) override;
        [[nodiscard]] std::shared_ptr<IBuffer> GetIndexBuffer() const override { return m_indexBuffer.first; }
        [[nodiscard]] std::optional<BufferDataType> GetIndexBufferType() const override { return m_indexBuffer.second; }

        void SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IBuffer> buffer,
                             const BufferBindingParams& binding) override;
        [[nodiscard]] std::shared_ptr<IBuffer> GetVertexBuffer(unsigned int index) const override;
        [[nodiscard]] std::optional<size_t> GetLastAttributeIndex() const noexcept override;
        [[nodiscard]] std::optional<BufferBindingParams> GetVertexBufferBinding(unsigned int index) const override;

        const VertexArrayDescriptor& GetVertexDescriptor() const override { return m_vertexDescriptor; }

    private:
        GLuint m_id;

        //we are sure that all buffer will be at least GlBuffer or derived types
        std::vector<std::pair<std::shared_ptr<IBuffer>, BufferBindingParams>> m_buffers;
        std::pair<std::shared_ptr<IBuffer>, std::optional<BufferDataType>> m_indexBuffer;

        VertexArrayDescriptor m_vertexDescriptor;
    };

} // namespace AT2::OpenGL41
#endif
