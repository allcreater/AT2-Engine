#pragma once

#include "BufferLayout.h"

namespace AT2
{
    // Just a container for pair "buffer + layout", could be replaced just to free function IBuffer::Commit or like this
    class StructuredBuffer : public IUniformReceiver
    {
    public:
        StructuredBuffer(std::shared_ptr<IBuffer> buffer, std::shared_ptr<const BufferLayout> bufferLayout);

        const std::shared_ptr<IBuffer>& GetBuffer() const { return m_buffer; }
        const std::shared_ptr<const BufferLayout>& GetBufferLayout() const { return m_bufferLayout; }

        void Commit(const std::function<void(IUniformsWriter&)>& operation) override;

    private:
        std::shared_ptr<IBuffer> m_buffer;
        std::shared_ptr<const BufferLayout> m_bufferLayout;
    };

}