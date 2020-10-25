#pragma once

#include <filesystem>

#include "../AT2.h"

namespace AT2::Resources
{
    class IDataSource
    {
    public:
        virtual ~IDataSource() = default;

        [[nodiscard]] virtual std::string GetName() const noexcept = 0;
        [[nodiscard]] virtual bool IsReloadable() const noexcept = 0;
        [[nodiscard]] virtual std::unique_ptr<std::istream> Load() = 0;

        std::vector<char> LoadBytes();
    };

    class FileDataSource final: public IDataSource
    {
    public:
        explicit FileDataSource(std::filesystem::path filename) : m_filename(std::move(filename)) {}

        const std::filesystem::path& getFilename() const noexcept { return m_filename; }
        void setFilename(std::filesystem::path _filename) { m_filename = std::move(_filename); }

        [[nodiscard]] std::string GetName() const noexcept override;
        [[nodiscard]] bool IsReloadable() const noexcept override;
        [[nodiscard]] std::unique_ptr<std::istream> Load() override;

    private:
        std::filesystem::path m_filename;
    };

    //Probably better to write something like "MemoryDataSource" with std::span
    class StringDataSource final : public IDataSource
    {
    public:
        explicit StringDataSource(std::string data, std::string name) : m_data(std::move(data)), m_name(std::move(name)) {}

        const std::string& getData() const noexcept { return m_data; }
        void setData(std::string _data) { m_data = std::move(_data); }

        [[nodiscard]] std::string GetName() const noexcept override;
        [[nodiscard]] bool IsReloadable() const noexcept override { return true; }
        [[nodiscard]] std::unique_ptr<std::istream> Load() override;

    private:
        std::string m_data;
        std::string m_name;
    };

    namespace Literals
    {
        std::unique_ptr<FileDataSource> inline operator""_FDS(const char* filename, size_t)
        {
            return std::make_unique<FileDataSource> (filename);
        }

        std::unique_ptr<StringDataSource> inline operator""_SDS(const char* string, size_t)
        {
            return std::make_unique<StringDataSource> (string, "");
        }

    }

} // namespace AT2::Resources