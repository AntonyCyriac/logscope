/**
 * @file correlation_engine.cpp
 */

#include "correlation_engine.hpp"

#include "foundation/hash.hpp"
#include "foundation/timestamp.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <map>
#include <regex>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace scope::workspace
{

namespace
{

constexpr std::int64_t kProximityWindowMs = 30'000;
constexpr std::size_t kMaxSuggestionsPerEvent = 50U;
constexpr std::size_t kMaxSuggestionsPerInvestigation = 500U;
constexpr std::size_t kMaxSummaryLength = 500U;

struct KeyPattern
{
    CorrelationKey key;
    std::vector<std::string> structuredFieldNames;
    std::regex messagePattern;
};

std::string trimWhitespace(const std::string& value)
{
    std::size_t start = 0U;

    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0)
    {
        ++start;
    }

    std::size_t end = value.size();

    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1U])) != 0)
    {
        --end;
    }

    return value.substr(start, end - start);
}

std::string formatHashHex(const std::uint64_t value)
{
    std::ostringstream output;
    output << std::hex << std::setw(16) << std::setfill('0') << value;

    return output.str();
}

std::pair<std::string, std::string> orderedEventPair(const std::string& first, const std::string& second)
{
    if (first <= second)
    {
        return {first, second};
    }

    return {second, first};
}

std::string makeSuggestionId(const std::string& investigationId, const std::string& sourceEventId,
                           const std::string& targetEventId, const CorrelationKey matchedKey)
{
    const auto [minEventId, maxEventId] = orderedEventPair(sourceEventId, targetEventId);

    std::ostringstream key;
    key << investigationId << '|' << minEventId << '|' << maxEventId << '|' << correlationKeyToString(matchedKey);

    return formatHashHex(foundation::hashString(key.str()));
}

std::optional<std::int64_t> parseEventTimestampMs(const TimelineEvent& event)
{
    const auto parsed = foundation::Timestamp::parse(event.timestamp);

    if (!parsed)
    {
        return std::nullopt;
    }

    return parsed->unixNanoseconds() / 1'000'000;
}

std::optional<int> lineRefForEvent(const TimelineEvent& event)
{
    if (event.source.lineNumber.has_value())
    {
        return static_cast<int>(*event.source.lineNumber);
    }

    return std::nullopt;
}

std::string formatTimeDelta(const std::int64_t deltaMs)
{
    const std::int64_t absoluteMs = deltaMs >= 0 ? deltaMs : -deltaMs;
    const double seconds = static_cast<double>(absoluteMs) / 1000.0;
    std::ostringstream output;
    output << std::fixed << std::setprecision(1) << seconds << 's';

    return output.str();
}

std::string buildSummary(const CorrelationKey matchedKey, const std::string& matchedValue,
                         const std::string& sourceArtifactName, const std::string& targetArtifactName,
                         const std::optional<std::int64_t> timeDeltaMs)
{
    std::ostringstream summary;
    summary << "Matched " << correlationKeyToString(matchedKey) << '=' << matchedValue << " across "
            << sourceArtifactName << " and " << targetArtifactName;

    if (timeDeltaMs.has_value())
    {
        summary << " (Δt " << formatTimeDelta(*timeDeltaMs) << ')';
    }

    std::string result = summary.str();

    if (result.size() > kMaxSummaryLength)
    {
        result.resize(kMaxSummaryLength);
    }

    return result;
}

const std::vector<KeyPattern>& keyPatterns()
{
    static const std::vector<KeyPattern> patterns = {
        {CorrelationKey::RequestId,
         {"request_id", "requestId", "http.request_id"},
         std::regex(R"(request[_-]?id[=:\s]+([^\s,;]+))", std::regex::ECMAScript | std::regex::icase)},
        {CorrelationKey::TraceId,
         {"trace_id", "traceId", "trace.id"},
         std::regex(R"(trace[_-]?id[=:\s]+([^\s,;]+))", std::regex::ECMAScript | std::regex::icase)},
        {CorrelationKey::SessionId,
         {"session_id", "sessionId"},
         std::regex(R"(session[_-]?id[=:\s]+([^\s,;]+))", std::regex::ECMAScript | std::regex::icase)},
        {CorrelationKey::TransactionId,
         {"transaction_id", "transactionId", "txn_id"},
         std::regex(R"(transaction[_-]?id[=:\s]+([^\s,;]+))", std::regex::ECMAScript | std::regex::icase)},
        {CorrelationKey::CorrelationId,
         {"correlation_id", "correlationId", "corr_id"},
         std::regex(R"(correlation[_-]?id[=:\s]+([^\s,;]+))", std::regex::ECMAScript | std::regex::icase)},
    };

    return patterns;
}

std::optional<std::string> extractStructuredValue(const TimelineEvent& event, const KeyPattern& pattern)
{
    for (const std::string& fieldName : pattern.structuredFieldNames)
    {
        const auto iterator = event.metadata.find(fieldName);

        if (iterator != event.metadata.end() && !iterator->second.empty())
        {
            return trimWhitespace(iterator->second);
        }
    }

    return std::nullopt;
}

std::optional<std::string> extractRegexValue(const std::string& message, const KeyPattern& pattern)
{
    std::smatch match;

    if (!std::regex_search(message, match, pattern.messagePattern) || match.size() < 2U)
    {
        return std::nullopt;
    }

    const std::string captured = trimWhitespace(match[1].str());

    if (captured.empty())
    {
        return std::nullopt;
    }

    return captured;
}

std::map<CorrelationKey, std::string> extractKeys(const TimelineEvent& event)
{
    std::map<CorrelationKey, std::string> extracted;

    for (const KeyPattern& pattern : keyPatterns())
    {
        std::optional<std::string> value = extractStructuredValue(event, pattern);

        if (!value.has_value())
        {
            value = extractRegexValue(event.message, pattern);
        }

        if (value.has_value() && !value->empty())
        {
            extracted.emplace(pattern.key, *value);
        }
    }

    return extracted;
}

bool proximityOk(const TimelineEvent& first, const TimelineEvent& second)
{
    const std::optional<std::int64_t> firstMs = parseEventTimestampMs(first);
    const std::optional<std::int64_t> secondMs = parseEventTimestampMs(second);

    if (!firstMs.has_value() || !secondMs.has_value())
    {
        return true;
    }

    const std::int64_t delta = *secondMs - *firstMs;

    return delta <= kProximityWindowMs && delta >= -kProximityWindowMs;
}

using UndirectedPair = std::pair<std::string, std::string>;

struct PairHash
{
    std::size_t operator()(const UndirectedPair& pair) const noexcept
    {
        std::uint64_t seed = foundation::hashString(pair.first);
        foundation::hashCombine(seed, foundation::hashString(pair.second));

        return static_cast<std::size_t>(seed);
    }
};

std::unordered_set<UndirectedPair, PairHash> collectLinkedEventPairs(const std::vector<EvidenceLink>& links)
{
    std::unordered_set<UndirectedPair, PairHash> pairs;

    for (const EvidenceLink& link : links)
    {
        if (link.source.eventId.empty() || link.target.eventId.empty())
        {
            continue;
        }

        pairs.insert(orderedEventPair(link.source.eventId, link.target.eventId));
    }

    return pairs;
}

bool suggestionInvolvesEvent(const CorrelationSuggestion& suggestion, const std::string& eventId)
{
    return suggestion.sourceEventId == eventId || suggestion.targetEventId == eventId;
}

} // namespace

CorrelationSuggestionListResult CorrelationEngine::computeSuggestions(
    const std::string& investigationId, const std::vector<TimelineEvent>& events,
    const std::vector<EvidenceLink>& links, const std::unordered_set<std::string>& dismissedSuggestionIds,
    CorrelationSuggestionQuery query)
{
    if (query.limit <= 0)
    {
        query.limit = 50;
    }

    if (query.limit > 50)
    {
        query.limit = 50;
    }

    if (query.offset < 0)
    {
        query.offset = 0;
    }

    std::unordered_map<std::string, const TimelineEvent*> eventsById;
    eventsById.reserve(events.size());

    for (const TimelineEvent& event : events)
    {
        eventsById.emplace(event.id, &event);
    }

    using IndexKey = std::pair<CorrelationKey, std::string>;
    std::map<IndexKey, std::vector<std::string>> index;

    for (const TimelineEvent& event : events)
    {
        for (const auto& [key, value] : extractKeys(event))
        {
            index[{key, value}].push_back(event.id);
        }
    }

    const std::unordered_set<UndirectedPair, PairHash> linkedPairs = collectLinkedEventPairs(links);
    std::vector<CorrelationSuggestion> candidates;
    candidates.reserve(128U);

    for (const auto& [indexKey, eventIds] : index)
    {
        if (eventIds.size() < 2U)
        {
            continue;
        }

        const CorrelationKey matchedKey = indexKey.first;
        const std::string& matchedValue = indexKey.second;

        for (std::size_t firstIndex = 0U; firstIndex < eventIds.size(); ++firstIndex)
        {
            for (std::size_t secondIndex = firstIndex + 1U; secondIndex < eventIds.size(); ++secondIndex)
            {
                const TimelineEvent* firstEvent = eventsById.at(eventIds[firstIndex]);
                const TimelineEvent* secondEvent = eventsById.at(eventIds[secondIndex]);

                if (firstEvent->artifactId == secondEvent->artifactId)
                {
                    continue;
                }

                const UndirectedPair pairKey = orderedEventPair(firstEvent->id, secondEvent->id);

                if (linkedPairs.count(pairKey) > 0U)
                {
                    continue;
                }

                if (!proximityOk(*firstEvent, *secondEvent))
                {
                    continue;
                }

                const auto [sourceEventId, targetEventId] = pairKey;
                const TimelineEvent* sourceEvent = eventsById.at(sourceEventId);
                const TimelineEvent* targetEvent = eventsById.at(targetEventId);

                CorrelationSuggestion suggestion;
                suggestion.id = makeSuggestionId(investigationId, sourceEventId, targetEventId, matchedKey);
                suggestion.sourceEventId = sourceEventId;
                suggestion.targetEventId = targetEventId;
                suggestion.matchedKey = matchedKey;
                suggestion.matchedValue = matchedValue;
                suggestion.sourceArtifactName = sourceEvent->source.artifactName;
                suggestion.targetArtifactName = targetEvent->source.artifactName;
                suggestion.sourceLineRef = lineRefForEvent(*sourceEvent);
                suggestion.targetLineRef = lineRefForEvent(*targetEvent);
                suggestion.ruleId = "exact_key_match";

                const std::optional<std::int64_t> sourceMs = parseEventTimestampMs(*sourceEvent);
                const std::optional<std::int64_t> targetMs = parseEventTimestampMs(*targetEvent);

                if (sourceMs.has_value() && targetMs.has_value())
                {
                    suggestion.timeDeltaMs = *targetMs - *sourceMs;
                }

                suggestion.summary = buildSummary(matchedKey, matchedValue, suggestion.sourceArtifactName,
                                                suggestion.targetArtifactName, suggestion.timeDeltaMs);

                if (dismissedSuggestionIds.count(suggestion.id) > 0U)
                {
                    continue;
                }

                candidates.push_back(std::move(suggestion));
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [&eventsById](const CorrelationSuggestion& left, const CorrelationSuggestion& right) {
                  const bool leftHasDelta = left.timeDeltaMs.has_value();
                  const bool rightHasDelta = right.timeDeltaMs.has_value();

                  if (leftHasDelta != rightHasDelta)
                  {
                      return leftHasDelta;
                  }

                  if (leftHasDelta && rightHasDelta)
                  {
                      const std::int64_t leftAbs = *left.timeDeltaMs >= 0 ? *left.timeDeltaMs : -(*left.timeDeltaMs);
                      const std::int64_t rightAbs =
                          *right.timeDeltaMs >= 0 ? *right.timeDeltaMs : -(*right.timeDeltaMs);

                      if (leftAbs != rightAbs)
                      {
                          return leftAbs < rightAbs;
                      }
                  }

                  const TimelineEvent* leftTarget = eventsById.at(left.targetEventId);
                  const TimelineEvent* rightTarget = eventsById.at(right.targetEventId);

                  return leftTarget->timestamp < rightTarget->timestamp;
              });

    std::unordered_map<std::string, std::size_t> involvementCounts;
    std::vector<CorrelationSuggestion> capped;
    capped.reserve(candidates.size());
    bool truncated = false;

    for (const CorrelationSuggestion& candidate : candidates)
    {
        if (capped.size() >= kMaxSuggestionsPerInvestigation)
        {
            truncated = true;
            break;
        }

        const std::size_t sourceCount = involvementCounts[candidate.sourceEventId];
        const std::size_t targetCount = involvementCounts[candidate.targetEventId];

        if (sourceCount >= kMaxSuggestionsPerEvent || targetCount >= kMaxSuggestionsPerEvent)
        {
            truncated = true;
            continue;
        }

        capped.push_back(candidate);
        ++involvementCounts[candidate.sourceEventId];
        ++involvementCounts[candidate.targetEventId];
    }

    if (capped.size() < candidates.size())
    {
        truncated = true;
    }

    std::vector<CorrelationSuggestion> filtered;
    filtered.reserve(capped.size());

    if (query.eventId.has_value())
    {
        for (const CorrelationSuggestion& suggestion : capped)
        {
            if (suggestionInvolvesEvent(suggestion, *query.eventId))
            {
                filtered.push_back(suggestion);
            }
        }
    }
    else
    {
        filtered = std::move(capped);
    }

    CorrelationSuggestionListResult result;
    result.total = static_cast<int>(filtered.size());
    result.limit = query.limit;
    result.offset = query.offset;
    result.truncated = truncated;

    const std::size_t start = static_cast<std::size_t>(query.offset);
    const std::size_t end = std::min(filtered.size(), start + static_cast<std::size_t>(query.limit));

    if (start < filtered.size())
    {
        result.suggestions.assign(filtered.begin() + static_cast<std::ptrdiff_t>(start),
                                 filtered.begin() + static_cast<std::ptrdiff_t>(end));
    }

    return result;
}

} // namespace scope::workspace
