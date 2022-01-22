#include "BufferLayout.h"
#include "BufferLayout.h"
#include "IO.hpp"

using namespace AT2;

BufferLayout::BufferLayout(std::vector<Field> fields)
: m_fields {std::move(fields)}
, m_fieldsByName {m_fields.size()}
{
    std::sort(m_fields.begin(), m_fields.end(), [](const Field& lhs, const Field& rhs) { return rhs.GetOffset() > lhs.GetOffset(); });

    { // Error check
        const auto it = std::adjacent_find(m_fields.begin(), m_fields.end(), [](const Field& lhs, const Field& rhs) {
            return rhs.GetOffset() < lhs.GetOffset() || rhs.GetOffset() + rhs.GetSize() < lhs.GetOffset() + lhs.GetSize();
        });

        if (it != m_fields.end())
            throw std::logic_error("Fields array must be non-overlapping");
    }

    using FieldsByNamePair = decltype(m_fieldsByName)::value_type;
    std::transform(m_fields.begin(), m_fields.end(), std::inserter(m_fieldsByName, m_fieldsByName.end()), [](const Field& field) {
        return FieldsByNamePair {field.GetName(), &field};
    });
}

namespace
{
    class StructuredBufferWriter : public StructuredBuffer::IStructuredDataWriter
    {
    public:
        StructuredBufferWriter(IBuffer& buffer, BufferLayout& layout) :
            m_buffer {buffer}, m_layout {layout}, m_mappedBuffer {m_buffer.Map(BufferUsage::ReadWrite)}
        {
        }

        ~StructuredBufferWriter() override { m_buffer.Unmap(); }

        void Write(std::string_view name, Uniform value) override { WriteImpl(name, value); }
        void Write(std::string_view name, UniformArray value) override { WriteImpl(name, value); }

    private:
        template <typename T>
        void WriteImpl(std::string_view name, const T& value)
        {
            std::visit(
                [&](const auto& x) {
                    if (auto* field = m_layout[name])
                        DataIO::Write(*field, m_mappedBuffer, x);
                },
                value);
        }

        IBuffer& m_buffer;
        BufferLayout& m_layout;
        std::span<std::byte> m_mappedBuffer;
    };
} // namespace

void StructuredBuffer::Commit( const std::function<void(IStructuredDataWriter&)>& operation )
{
    StructuredBufferWriter writer {*m_buffer, *m_bufferLayout};
    operation(writer);
}
