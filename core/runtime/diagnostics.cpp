/**
 * @file diagnostics.cpp
 * @brief Diagnostics implementation.
 */

#include "diagnostics.hpp"

#include <iostream>

namespace scope::runtime
{

Diagnostics& Diagnostics::instance()
{
    static Diagnostics diagnostics;

    return diagnostics;
}

void Diagnostics::log(LogLevel level, std::string_view message)
{
    const char* label = "INFO";

    switch (level)
    {
    case LogLevel::Debug:
        label = "DEBUG";
        break;
    case LogLevel::Info:
        label = "INFO";
        break;
    case LogLevel::Warning:
        label = "WARN";
        break;
    case LogLevel::Error:
        label = "ERROR";
        break;
    }

    std::cerr << '[' << label << "] " << message << std::endl;
}

} // namespace scope::runtime
