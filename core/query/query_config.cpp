/**
 * @file query_config.cpp
 */

#include "query_config.hpp"

#include "foundation/string.hpp"
#include "query_parser.hpp"

namespace scope::query
{

foundation::Result<bool> validateQueryConfiguration(const runtime::Configuration& configuration) noexcept
{
    for (const std::string& key : configuration.keys())
    {
        if (!foundation::startsWith(key, savedFilterKeyPrefix))
        {
            continue;
        }

        const std::string suffix = key.substr(std::string_view(savedFilterKeyPrefix).size());

        if (foundation::isBlank(suffix))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Saved filter key must include a name after query.saved."));
        }

        const auto value = configuration.get(key);

        if (!value || foundation::isBlank(*value))
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "Saved filter '" + suffix + "' must not be empty."));
        }

        const auto parsed = parseFilterQuery(*value);

        if (!parsed)
        {
            return foundation::Result<bool>(foundation::Error(
                foundation::ErrorCode::InvalidArgument,
                "Invalid saved filter '" + suffix + "': " + parsed.error().message()));
        }
    }

    return foundation::Result<bool>(true);
}

} // namespace scope::query
