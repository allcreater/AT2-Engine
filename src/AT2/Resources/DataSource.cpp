#include "DataSource.h"

#include <fstream>
#include <sstream>

using namespace AT2;
using namespace AT2::Resources;

bool FileDataSource::IsReloadable() const noexcept
{
    try
    {
        return exists(filename);
    }
    catch (...)
    {
        return false;
    }
}

std::unique_ptr<std::istream> FileDataSource::Load()
{
    return std::make_unique<std::ifstream> (filename);
}

std::unique_ptr<std::istream> StringDataSource::Load()
{
    return std::make_unique<std::istringstream>(data);
}
