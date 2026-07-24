/**
 * @file query_cache_key.cpp
 */

#include "query_cache_key.hpp"

#include <sstream>

#include "foundation/hash.hpp"
#include "query_node.hpp"

namespace scope::storage
{

std::string computeQueryCacheKey(const std::string& fingerprint, const std::string& canonicalFilter,
                                 const int schemaVersion) noexcept
{
    std::ostringstream material;
    material << fingerprint << '\n' << canonicalFilter << '\n' << schemaVersion;

    const std::uint64_t digest = foundation::hashString(material.str());

    std::ostringstream formatted;
    formatted << std::hex << digest;

    return formatted.str();
}

std::string canonicalFilterText(const query::QueryNode& filterNode) noexcept
{
    if (!filterNode.isActive())
    {
        return "MATCH_ALL";
    }

    return filterNode.toString();
}

} // namespace scope::storage
