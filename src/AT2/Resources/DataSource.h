#pragma once

#include <filesystem>

#include "../AT2.h"

namespace AT2::Resources
{
    class IDataSource
    {
    public:
        virtual ~IDataSource() = default;

        [[nodiscard]] virtual bool IsReloadable() const noexcept = 0;
        [[nodiscard]] virtual std::unique_ptr<std::istream> Load() = 0;
    };

    class FileDataSource final: public IDataSource
    {
    public:
        explicit FileDataSource(std::filesystem::path filename) : filename(std::move(filename)) {}

        const std::filesystem::path& getFilename() const noexcept { return filename; }
        void setFilename(std::filesystem::path _filename) { filename = std::move(_filename); }

        [[nodiscard]] bool IsReloadable() const noexcept override;
        [[nodiscard]] std::unique_ptr<std::istream> Load() override;

    private:
        std::filesystem::path filename;
    };

    //Probably better to write something like "MemoryDataSource" with std::span
    class StringDataSource final : public IDataSource
    {
    public:
        explicit StringDataSource(std::string data) : data(std::move(data)) {}

        const std::string& getData() const noexcept { return data; }
        void setData(std::string _data) { data = std::move(_data); }

        [[nodiscard]] bool IsReloadable() const noexcept override { return true; }
        [[nodiscard]] std::unique_ptr<std::istream> Load() override;

    private:
        std::string data;
    };

    namespace Literals
    {
        std::unique_ptr<FileDataSource> inline operator""_FDS(const char* filename, size_t)
        {
            return std::make_unique<FileDataSource> (filename);
        }

        std::unique_ptr<StringDataSource> inline operator""_SDS(const char* string, size_t)
        {
            return std::make_unique<StringDataSource> (string);
        }

    }

} // namespace AT2::Resources