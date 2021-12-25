#ifndef AT2_LOG_HEADER_H
#define AT2_LOG_HEADER_H

#include <iostream>

namespace AT2
{
    namespace Log
    {
        std::ostream& Error();
        std::ostream& Warning();
        std::ostream& Info();
        std::ostream& Debug();
    } // namespace Log
} // namespace AT2

#endif