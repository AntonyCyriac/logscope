/**
 * @file text_matcher.cpp
 * @brief Substring matching implementation.
 */

#include "text_matcher.hpp"

#include "foundation/string.hpp"

namespace scope::search
{

bool textContains(const std::string_view haystack, const std::string_view needle,
                  const CaseSensitivity caseSensitivity) noexcept
{
    if (needle.empty())
    {
        return true;
    }

    if (haystack.empty())
    {
        return false;
    }

    if (caseSensitivity == CaseSensitivity::Sensitive)
    {
        return haystack.find(needle) != std::string_view::npos;
    }

    const std::string loweredHaystack = foundation::toLower(haystack);
    const std::string loweredNeedle = foundation::toLower(needle);

    return loweredHaystack.find(loweredNeedle) != std::string::npos;
}

} // namespace scope::search
