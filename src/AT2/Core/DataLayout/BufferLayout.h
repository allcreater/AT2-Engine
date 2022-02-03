#pragma once

#include <AT2.h>
#include <span>
#include <unordered_map>

namespace AT2
{
    class ArrayAttributes final
    {
    public:
        ArrayAttributes() = default;
        ArrayAttributes(unsigned int elementsCount, unsigned int arrayStride) : elementsCount {elementsCount}, m_arrayStride {arrayStride} {}

        [[nodiscard]] unsigned int GetLength() const noexcept { return elementsCount; }
        [[nodiscard]] unsigned int GetStride() const noexcept { return m_arrayStride; }

        [[nodiscard]] operator bool() const { return elementsCount != 0 && m_arrayStride != 0; }

    private:
        unsigned int elementsCount = 0, m_arrayStride = 0;
    };


    class Field final
    {
    public:
        Field(std::string name, size_t offset, ArrayAttributes arrayAttributes, unsigned int matrixStride, size_t size = std::dynamic_extent ) :
            m_name {std::move(name)},
            m_offset {offset}, m_size {size}, m_arrayAttributes {arrayAttributes}, m_matrixStride {matrixStride}
        {
        }

        [[nodiscard]] const std::string& GetName() const & noexcept { return m_name; }
        [[nodiscard]] size_t GetOffset() const noexcept { return m_offset; }

        //optional attributes, zero means "no attribute"
    	[[nodiscard]] size_t GetSize() const noexcept { return m_size != std::dynamic_extent ? m_size : 0; }
        [[nodiscard]] ArrayAttributes GetArrayAttributes() const noexcept { return m_arrayAttributes; }
    	[[nodiscard]] unsigned int GetMatrixStride() const noexcept { return m_matrixStride; }

        [[nodiscard]] std::span<std::byte> Map(std::span<std::byte> data) noexcept { return data.subspan(m_offset, m_size); }

    private:
        std::string m_name;
        size_t m_offset;
        
        size_t m_size;
        ArrayAttributes m_arrayAttributes;
        unsigned int m_matrixStride;
    };


    class BufferLayout
    {
    public:
        BufferLayout(std::vector<Field> fields, size_t alignment = 0);

        const Field*          operator[](std::string_view name) const;

        size_t GetAlignment() const noexcept { return m_alignment; }
        
        std::optional<size_t> GetFieldNumber(std::string_view name) const;

        std::span<const Field> GetFields() const noexcept { return m_fields; }
        
        size_t GetSufficientSize() const;
        
    private:
        size_t GetSufficientSizeInternal() const;

    private:
        size_t m_alignment;
        std::vector<Field> m_fields;
        std::unordered_map<std::string_view, const Field*> m_fieldsByName;
    };

}
