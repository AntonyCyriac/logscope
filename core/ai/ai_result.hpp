/**
 * @file ai_result.hpp
 * @brief Structured AI assistant output types (M13).
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace scope::ai
{

/**
 * @brief A single evidence citation for AI output.
 */
struct AiEvidence
{
    std::uint64_t lineNumber{0U};
    std::string text;
};

/**
 * @brief Investigation summary produced by an AI provider.
 */
struct AiSummary
{
    std::string summary;
    std::string reasoning;
    std::vector<AiEvidence> evidence;
    std::string confidence;
    std::vector<std::string> suggestedActions;
};

/**
 * @brief A single anomaly hint.
 */
struct AiAnomalyHint
{
    std::string message;
    std::string severity;
};

/**
 * @brief Bounded investigation context passed to providers.
 */
struct AiInvestigationContext
{
    std::string sourceSummary;
    std::vector<AiEvidence> sampleLines;
    std::uint64_t matchCount{0U};
    std::string searchQuerySummary;
    std::uint64_t indexedLineCount{0U};
    std::uint64_t truncatedLineCount{0U};
    std::size_t repeatedErrorPatternCount{0U};
    std::string topRepeatedErrorKey;
    std::uint64_t topRepeatedErrorCount{0U};
};

/**
 * @brief Bounded analytics signals passed to providers for hints.
 */
struct AiAnalyticsContext
{
    bool hasSpike{false};
    std::string spikeVerdict;
    std::size_t clusterCount{0U};
    std::string topClusterMessage;
};

} // namespace scope::ai
