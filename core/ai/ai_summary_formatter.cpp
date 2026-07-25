/**
 * @file ai_summary_formatter.cpp
 */

#include "ai_summary_formatter.hpp"

#include <sstream>

namespace scope::ai
{

std::string formatAiSummary(const AiSummary& summary)
{
    std::ostringstream output;

    output << "Summary\n";
    output << summary.summary << "\n\n";

    if (!summary.reasoning.empty())
    {
        output << "Reasoning\n";
        output << summary.reasoning << "\n\n";
    }

    if (!summary.evidence.empty())
    {
        output << "Evidence\n";

        for (const auto& item : summary.evidence)
        {
            output << "- Line " << item.lineNumber << ": " << item.text << "\n";
        }

        output << "\n";
    }

    if (!summary.confidence.empty())
    {
        output << "Confidence: " << summary.confidence << "\n\n";
    }

    if (!summary.suggestedActions.empty())
    {
        output << "Suggested actions\n";

        for (const auto& action : summary.suggestedActions)
        {
            output << "- " << action << "\n";
        }
    }

    return output.str();
}

} // namespace scope::ai
