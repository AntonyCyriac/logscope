/**
 * @file search_config.cpp
 * @brief Search configuration validation implementation.
 */

#include "search_config.hpp"

#include "foundation/string.hpp"
#include "search_query_parser.hpp"

namespace scope::search
{

foundation::Result<bool> validateSearchConfiguration(const runtime::Configuration& configuration) noexcept
{
    for (const std::string& key : configuration.keys())
    {
        if (!foundation::startsWith(key, savedSearchKeyPrefix))
        {
            continue;
        }

        const std::string suffix = key.substr(std::string_view(savedSearchKeyPrefix).size());

        if (foundation::isBlank(suffix))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Saved search key must include a name after search.saved."));
        }

        const auto value = configuration.get(key);

        if (!value || foundation::isBlank(*value))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Saved search '" + suffix + "' must not be empty."));
        }

        const auto parsed = parseSearchQuery(*value);

        if (!parsed)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Invalid saved search '" + suffix + "': " + parsed.error().message()));
        }
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::search
