#pragma once

#include "VertexBuffer.h"

namespace AT2::Metal
{

    class VertexArray : public IVertexArray
    {
    public:
        NON_COPYABLE_OR_MOVABLE(VertexArray)

        VertexArray(const IRendererCapabilities& rendererCapabilities);
        ~VertexArray() override;

    public:
        void Bind() override;
        [[nodiscard]] unsigned int GetId() const noexcept override { return m_id; }

        void SetIndexBuffer(std::shared_ptr<IVertexBuffer> buffer, BufferDataType type) override;
        [[nodiscard]] std::shared_ptr<IVertexBuffer> GetIndexBuffer() const override { return m_indexBuffer.first; }
        [[nodiscard]] std::optional<BufferDataType> GetIndexBufferType() const override { return m_indexBuffer.second; }

        void SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IVertexBuffer> buffer,
                             const BufferBindingParams& binding) override;
        [[nodiscard]] std::shared_ptr<IVertexBuffer> GetVertexBuffer(unsigned int index) const override;
        [[nodiscard]] std::optional<BufferBindingParams> GetVertexBufferBinding(unsigned int index) const override;

    private:
        GLuint m_id;

        //we are sure that all buffer will be at least GlVertexBuffer or derived types
        std::vector<std::pair<std::shared_ptr<IVertexBuffer>, BufferBindingParams>> m_buffers;
        std::pair<std::shared_ptr<IVertexBuffer>, std::optional<BufferDataType>> m_indexBuffer;
    };

} // namespace AT2::Metal
