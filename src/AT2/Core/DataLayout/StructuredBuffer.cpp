#include "StructuredBuffer.h"
#include "IO.hpp"

using namespace AT2;

namespace
{
    class StructuredBufferWriter : public IUniformsWriter
    {
    public:
        StructuredBufferWriter(IBuffer& buffer, const BufferLayout& layout) :
            m_buffer {buffer}, m_layout {layout}, m_mappedBuffer {m_buffer.Map(BufferOperationFlags::Write)}
        {
        }

        ~StructuredBufferWriter() override { m_buffer.Unmap(); }

        void Write(std::string_view name, Uniform value) override { WriteImpl(name, value); }
        void Write(std::string_view name, UniformArray value) override { WriteImpl(name, value); }
        void Write(std::string_view name, std::shared_ptr<ITexture> value) override
        {
            throw AT2NotImplementedException("StructuredBufferWriter::Write(Texture)");
        }
        void Write(std::string_view name, std::shared_ptr<IBuffer> value) override
        {
            throw AT2NotImplementedException("StructuredBufferWriter::Write(Buffer)");
        }

    private:
        template <typename T>
        void WriteImpl(std::string_view name, const T& value)
        {
            std::visit(
                [&](const auto& x) {
                    if (auto* field = m_layout[name])
                        DataIO::Write(*field, m_mappedBuffer.subspan(field->GetOffset()), x);
                },
                value);
        }

        IBuffer& m_buffer;
        const BufferLayout& m_layout;
        std::span<std::byte> m_mappedBuffer;
    };
} // namespace

StructuredBuffer::StructuredBuffer(std::shared_ptr<IBuffer> buffer, std::shared_ptr<const BufferLayout> bufferLayout) :
    m_buffer {std::move(buffer)}, m_bufferLayout {std::move(bufferLayout)}
{
    if (!m_buffer)
        throw AT2::AT2BufferException("StructuredBuffer must be initialized with buffer");

    if (!m_bufferLayout)
        throw AT2::AT2BufferException("StructuredBuffer must contain layout");

    m_buffer->ReserveSpace(m_bufferLayout->GetSufficientSize());
}

void StructuredBuffer::Commit(const std::function<void(IUniformsWriter&)>& operation)
{
    StructuredBufferWriter writer {*m_buffer, *m_bufferLayout};
    operation(writer);
}