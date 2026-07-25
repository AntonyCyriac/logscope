/**
 * @file ai_context_builder.cpp
 */

#include "ai_context_builder.hpp"

#include <algorithm>

namespace scope::ai
{

namespace
{

std::string evidenceText(const analysis::IndexedLine& line)
{
    if (!line.messageExcerpt.empty())
    {
        return line.messageExcerpt;
    }

    return line.contentExcerpt;
}

} // namespace

AiInvestigationContext buildInvestigationContext(
    const AiConfig& config,
    const investigation::InvestigationView& view,
    const investigation::InvestigationResult& result)
{
    AiInvestigationContext context;

    context.sourceSummary = view.summary();
    context.matchCount = static_cast<std::uint64_t>(result.matchingLines.size());
    context.searchQuerySummary = result.searchQuerySummary;
    context.indexedLineCount = result.indexedLineCount;
    context.truncatedLineCount = result.truncatedLineCount;

    const auto& repeatedErrors = result.correlations.repeatedErrors();
    context.repeatedErrorPatternCount = repeatedErrors.size();

    if (!repeatedErrors.empty())
    {
        context.topRepeatedErrorKey = repeatedErrors[0].key;
        context.topRepeatedErrorCount = repeatedErrors[0].count;
    }

    const std::size_t sampleCount = std::min(
        static_cast<std::size_t>(config.maxContextLines), result.matchingLines.size());

    context.sampleLines.reserve(sampleCount);

    for (std::size_t index = 0U; index < sampleCount; ++index)
    {
        const auto& line = result.matchingLines[index];

        AiEvidence evidence;
        evidence.lineNumber = line.lineNumber;
        evidence.text = evidenceText(line);
        context.sampleLines.push_back(std::move(evidence));
    }

    return context;
}

} // namespace scope::ai
