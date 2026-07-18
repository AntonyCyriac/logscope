/**
 * @file diagnostics.cpp
 * @brief Diagnostics implementation.
 */

#include "diagnostics.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>

#include "configuration.hpp"
#include "foundation/error.hpp"

namespace scope::runtime
{

namespace
{

std::string toLower(std::string_view value)
{
    std::string lowered(value);

    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    return lowered;
}

} // namespace

const char* toString(LogLevel level) noexcept
{
    switch (level)
    {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    }

    return "INFO";
}

foundation::Result<LogLevel> parseLogLevel(std::string_view value)
{
    const std::string lowered = toLower(value);

    if (lowered == "debug")
    {
        return foundation::Result<LogLevel>(LogLevel::Debug);
    }

    if (lowered == "info")
    {
        return foundation::Result<LogLevel>(LogLevel::Info);
    }

    if (lowered == "warn" || lowered == "warning")
    {
        return foundation::Result<LogLevel>(LogLevel::Warning);
    }

    if (lowered == "error")
    {
        return foundation::Result<LogLevel>(LogLevel::Error);
    }

    return foundation::Result<LogLevel>(
        foundation::Error(foundation::ErrorCode::InvalidArgument, "Invalid log level."));
}

Diagnostics& Diagnostics::instance()
{
    static Diagnostics diagnostics;

    return diagnostics;
}

LogLevel Diagnostics::minLevel() const noexcept
{
    return m_minLevel;
}

void Diagnostics::setMinLevel(LogLevel level) noexcept
{
    m_minLevel = level;
}

void Diagnostics::setOutputStream(std::ostream* stream) noexcept
{
    m_outputStream = stream;
}

bool Diagnostics::applyConfiguration(const Configuration& configuration)
{
    if (configuration.has("log.level"))
    {
        const auto levelResult = configuration.get("log.level");

        if (!levelResult)
        {
            return false;
        }

        const auto parsedLevel = parseLogLevel(*levelResult);

        if (!parsedLevel)
        {
            return false;
        }

        setMinLevel(*parsedLevel);

        return true;
    }

    if (const char* environmentLevel = std::getenv("SCOPE_LOG_LEVEL"))
    {
        const auto parsedLevel = parseLogLevel(environmentLevel);

        if (!parsedLevel)
        {
            return false;
        }

        setMinLevel(*parsedLevel);
    }

    return true;
}

void Diagnostics::log(LogLevel level, std::string_view message)
{
    log(level, "", message);
}

void Diagnostics::log(LogLevel level, std::string_view category, std::string_view message)
{
    if (!shouldLog(level))
    {
        return;
    }

    write(level, category, message);
}

bool Diagnostics::shouldLog(LogLevel level) const noexcept
{
    return static_cast<int>(level) >= static_cast<int>(m_minLevel);
}

void Diagnostics::write(LogLevel level, std::string_view category, std::string_view message)
{
    std::ostream& output = m_outputStream != nullptr ? *m_outputStream : std::cerr;

    output << '[' << toString(level) << ']';

    if (!category.empty())
    {
        output << " [" << category << ']';
    }

    output << ' ' << message << std::endl;
}

} // namespace scope::runtime
