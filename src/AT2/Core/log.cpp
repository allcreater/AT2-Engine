#include "log.h"

using namespace AT2::Log;


std::ostream& AT2::Log::Error()
{
    return std::cout << std::endl << "Error: ";
}

std::ostream& AT2::Log::Warning()
{
    return std::cout << std::endl << "Warning: ";
}

std::ostream& AT2::Log::Info()
{
    return std::cout << std::endl << "Info: ";
}
std::ostream& AT2::Log::Debug()
{
    return std::cout << std::endl << "Debug: ";
}
