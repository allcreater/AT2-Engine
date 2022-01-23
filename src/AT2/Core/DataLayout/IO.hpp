#pragma once

#include "BufferLayout.h"

namespace AT2::DataIO
{
    template <typename T>
    void CheckSize(const Field& field, std::span<const std::byte> data)
    {
        if (field.GetSize() && (sizeof(T) != field.GetSize() || data.size() < sizeof(T)))
            throw std::length_error("Field size is not equals to type size");
    }

    template <typename T>
    struct Reader
    {
        using ResultType = T;
        ResultType operator()(const Field& field, std::span<const std::byte> data)
        {
            CheckSize<ResultType>(field);

            T object;
            std::memcpy(&object, data.data(), sizeof(T));
            return object;
        }
    };

    //TODO: reader for matrices and span

    template <typename T>
    struct Writer
    {
        using Type = T;

        void operator()(const Field& field, std::span<std::byte> data, const Type& object)
        {
            CheckSize<Type>(field, data);

            std::memcpy(data.data(), &object, sizeof(T));
        }
    };

    template <typename T, glm::length_t C, glm::length_t R, glm::qualifier Q>
    struct Writer<glm::mat<C, R, T, Q>>
    {
        using Type = glm::mat<C, R, T, Q>;
        void operator()(const Field& field, std::span<std::byte> data, const Type& object)
        {
            CheckSize<Type>(field, data);

            //TODO row/column-based matrices
            if (!field.GetMatrixStride() || field.GetMatrixStride() < sizeof(object[0]))
                throw std::length_error("Matrix stride is not set or less than actual matrix row length");
            
            for (glm::length_t i = 0; i < C; ++i)
                std::memcpy(data.data() + i * field.GetMatrixStride(), glm::value_ptr(object[i]), sizeof(object[0]));
        }
    };

    template <typename T, size_t Extent>
    struct Writer<std::span<T, Extent>>
    {
        using Type = std::span<T, Extent>;
        void operator()(const Field& field, std::span<std::byte> data, const Type& span)
        {
            CheckSize<Type>(field, data);

            const auto arrayAttributes = field.GetArrayAttributes();
            if (!arrayAttributes)
                throw std::length_error("Array attributes must be set for write");

            if (arrayAttributes.GetStride() < sizeof(T))
                throw std::length_error("Array stride is less than actual element size");

            if (arrayAttributes.GetLength() != span.size())
                throw std::length_error("Array size doesn't match input array's size");

            Writer<T> writer;
            for (size_t i = 0; i < span.size(); ++i)
                writer(field, data, span[i]);
        }
    };

    template <typename T>
    void Write(const Field& field, std::span<std::byte> data, const T& object)
    {
        Writer<T> {}(field, data, object);
    }

} // namespace DataIO