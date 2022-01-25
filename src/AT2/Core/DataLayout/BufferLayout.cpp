#include "BufferLayout.h"

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
