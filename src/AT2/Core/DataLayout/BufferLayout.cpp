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

const Field* BufferLayout::operator[]( std::string_view name ) const
{
	const auto it = m_fieldsByName.find(name);
	return it != m_fieldsByName.end() ? it->second : nullptr;
}

std::optional<size_t> BufferLayout::GetFieldNumber( std::string_view name ) const
{
	const auto it = m_fieldsByName.find(name);
	return it != m_fieldsByName.end() ? std::optional {std::distance(m_fields.data(), it->second)} : std::nullopt;
}

size_t BufferLayout::GetSufficientSize() const
{
    if (m_fields.empty())
        return 0;

    const auto& lastField = m_fields.back();
    if (lastField.GetSize())
		return m_fields.back().GetOffset() + m_fields.back().GetSize();

	//TODO: precious calculation. Unfortunately reflection APIs don't report actual size so that we must guess :(
    return m_fields.back().GetOffset() + [&]() -> size_t {
        if (lastField.GetArrayAttributes())
            return static_cast<size_t>(lastField.GetArrayAttributes().GetLength()) * lastField.GetArrayAttributes().GetStride();

        if (lastField.GetMatrixStride())
            return static_cast<size_t>(lastField.GetMatrixStride()) * 4; //we don't support matrices more than 4x4

        //assert(false);
        return 256;
    }();
}


namespace
{
    class StructuredBufferWriter : public IUniformContainer::IUniformsWriter
    {
    public:
        StructuredBufferWriter(IBuffer& buffer, const BufferLayout& layout) :
            m_buffer {buffer}, m_layout {layout}, m_mappedBuffer {m_buffer.Map(BufferUsage::ReadWrite)}
        {
        }

        ~StructuredBufferWriter() override { m_buffer.Unmap(); }

        void Write(std::string_view name, Uniform value) override { WriteImpl(name, value); }
        void Write(std::string_view name, UniformArray value) override { WriteImpl(name, value); }
        void Write(std::string_view name, std::shared_ptr<ITexture> value) override { throw AT2NotImplementedException("StructuredBufferWriter::Write(Texture)");
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

StructuredBuffer::StructuredBuffer( std::shared_ptr<IBuffer> buffer, std::shared_ptr<const BufferLayout> bufferLayout ):
	m_buffer {std::move(buffer)}, m_bufferLayout {std::move(bufferLayout)}
{
    if (!m_buffer)
        throw AT2::AT2BufferException("StructuredBuffer must be initialized with buffer");

	if (!m_bufferLayout)
        throw AT2::AT2BufferException("StructuredBuffer must contain layout");

	m_buffer->ReserveSpace(m_bufferLayout->GetSufficientSize());
}

void StructuredBuffer::Commit( const std::function<void(IUniformContainer::IUniformsWriter&)>& operation )
{
    StructuredBufferWriter writer {*m_buffer, *m_bufferLayout};
    operation(writer);
}

void StructuredBuffer::Bind(IStateManager& stateManager) const
{
    stateManager.BindBuffer(m_bindingPoint, m_buffer);
}