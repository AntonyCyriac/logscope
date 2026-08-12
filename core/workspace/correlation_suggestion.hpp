/**
 * @file correlation_suggestion.hpp
 * @brief Ephemeral correlation suggestion types (Story 6 / v2.8.0).
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace scope::workspace
{

/**
 * @brief Exact-match correlation keys supported in v1.
 */
enum class CorrelationKey
{
    RequestId,
    TraceId,
    SessionId,
    TransactionId,
    CorrelationId
};

/**
 * @brief Ephemeral cross-artifact connection proposal (not persisted).
 */
struct CorrelationSuggestion
{
    std::string id;
    std::string sourceEventId;
    std::string targetEventId;
    CorrelationKey matchedKey;
    std::string matchedValue;
    std::string sourceArtifactName;
    std::string targetArtifactName;
    std::optional<int> sourceLineRef;
    std::optional<int> targetLineRef;
    std::optional<std::int64_t> timeDeltaMs;
    std::string ruleId;
    std::string summary;
};

/**
 * @brief Query options for listing correlation suggestions.
 */
struct CorrelationSuggestionQuery
{
    std::optional<std::string> eventId;
    int limit = 50;
    int offset = 0;
};

/**
 * @brief Paginated correlation suggestion list result.
 */
struct CorrelationSuggestionListResult
{
    std::vector<CorrelationSuggestion> suggestions;
    int total = 0;
    int limit = 50;
    int offset = 0;
    bool truncated = false;
};

[[nodiscard]] std::string correlationKeyToString(CorrelationKey key);

[[nodiscard]] std::optional<CorrelationKey> parseCorrelationKey(const std::string& value);

} // namespace scope::workspace
