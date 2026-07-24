/**
 * @file query_cache_key.hpp
 * @brief Cache key derivation for persisted filter results.
 */

#pragma once

#include <string>

namespace scope::query
{

class QueryNode;

} // namespace scope::query

namespace scope::storage
{

/**
 * @brief Computes a stable cache key from fingerprint, canonical filter AST, and schema version.
 */
[[nodiscard]] std::string computeQueryCacheKey(const std::string& fingerprint,
                                               const std::string& canonicalFilter,
                                               int schemaVersion) noexcept;

/**
 * @brief Returns the canonical filter text used for cache keys.
 */
[[nodiscard]] std::string canonicalFilterText(const query::QueryNode& filterNode) noexcept;

} // namespace scope::storage
