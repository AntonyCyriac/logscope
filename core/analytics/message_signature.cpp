/**
 * @file message_signature.cpp
 * @brief Cluster signature normalization.
 */

#include "message_signature.hpp"

#include <cctype>

#include "field_summary.hpp"
#include "foundation/string.hpp"

namespace scope::analytics
{

std::string normalizeClusterSignature(const std::string_view message)
{
    const std::string excerpt = analysis::normalizeMessageExcerpt(message);

    if (excerpt.empty())
    {
        return {};
    }

    std::string normalized;
    normalized.reserve(excerpt.size());

    bool inDigitRun = false;
    bool previousWasSpace = false;

    for (const char character : excerpt)
    {
        if (std::isdigit(static_cast<unsigned char>(character)) != 0)
        {
            if (!inDigitRun)
            {
                normalized.push_back('N');
                inDigitRun = true;
            }

            previousWasSpace = false;

            continue;
        }

        inDigitRun = false;

        if (std::isspace(static_cast<unsigned char>(character)) != 0)
        {
            if (!previousWasSpace && !normalized.empty())
            {
                normalized.push_back(' ');
                previousWasSpace = true;
            }

            continue;
        }

        normalized.push_back(character);
        previousWasSpace = false;
    }

    return foundation::trim(normalized);
}

} // namespace scope::analytics
