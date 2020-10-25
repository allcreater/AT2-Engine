#include "DataSource.h"

#include <fstream>
#include <sstream>

using namespace AT2;
using namespace AT2::Resources;

std::string FileDataSource::GetName() const noexcept
{
    return m_filename.filename().string();
}

bool FileDataSource::IsReloadable() const noexcept
{
    try
    {
        return exists(m_filename);
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<std::istream> FileDataSource::Load()
{
    return std::make_unique<std::ifstream>(m_filename, std::ios::in | std::ios::binary);
}


std::string StringDataSource::GetName() const noexcept
{
    return m_name;
}

std::unique_ptr<std::istream> StringDataSource::Load()
{
    return std::make_unique<std::istringstream>(m_data, std::ios::in | std::ios::binary);
}

std::vector<char> IDataSource::LoadBytes()
{
    const auto stream = Load();
    return {std::istreambuf_iterator<char> {*stream}, std::istreambuf_iterator<char> {}};
}