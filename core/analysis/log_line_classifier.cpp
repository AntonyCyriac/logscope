/**
 * @file log_line_classifier.cpp
 * @brief Log line level detection implementation.
 */

#include "log_line_classifier.hpp"

#include "foundation/string.hpp"

namespace scope::analysis
{

DetectedLogLevel detectLogLevel(const std::string_view line) noexcept
{
    if (foundation::isBlank(line))
    {
        return DetectedLogLevel::Blank;
    }

    const std::string lowered = foundation::toLower(line);

    if (lowered.find("error") != std::string::npos)
    {
        return DetectedLogLevel::Error;
    }

    if (lowered.find("warning") != std::string::npos || lowered.find(" warn") != std::string::npos)
    {
        return DetectedLogLevel::Warn;
    }

    if (lowered.find("info") != std::string::npos)
    {
        return DetectedLogLevel::Info;
    }

    return DetectedLogLevel::Other;
}

} // namespace scope::analysis
