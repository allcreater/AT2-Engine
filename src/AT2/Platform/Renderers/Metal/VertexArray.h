#pragma once

#include "Buffer.h"

namespace AT2::Metal
{

    class VertexArray : public IVertexArray
    {
    public:
        NON_COPYABLE_OR_MOVABLE(VertexArray)

        VertexArray(const IRendererCapabilities& rendererCapabilities);
        ~VertexArray() override;

    public:
        [[nodiscard]] unsigned int GetId() const noexcept override { return 0; }

        void SetIndexBuffer(std::shared_ptr<IBuffer> buffer, BufferDataType type) override;
        [[nodiscard]] std::shared_ptr<IBuffer> GetIndexBuffer() const override { return m_indexBuffer.first; }
        [[nodiscard]] std::optional<BufferDataType> GetIndexBufferType() const override { return m_indexBuffer.second; }

        void SetAttributeBinding(unsigned int attributeIndex, std::shared_ptr<IBuffer> buffer, const BufferBindingParams& binding) override;
        [[nodiscard]] std::shared_ptr<IBuffer> GetVertexBuffer(unsigned int index) const override;
        [[nodiscard]] std::optional<size_t> GetLastAttributeIndex() const noexcept override;
        [[nodiscard]] std::optional<BufferBindingParams> GetVertexBufferBinding(unsigned int index) const override;

        MtlPtr<MTL::VertexDescriptor> GetVertexDescriptor() { return m_vertexDescriptor; }
        
    private:
        //we are sure that all buffer will be at least GlVertexBuffer or derived types
        std::vector<std::pair<std::shared_ptr<IBuffer>, BufferBindingParams>> m_buffers;
        std::pair<std::shared_ptr<IBuffer>, std::optional<BufferDataType>> m_indexBuffer;
        
        MtlPtr<MTL::VertexDescriptor> m_vertexDescriptor;
    };

} // namespace AT2::Metal
