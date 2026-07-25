/**
 * @file ai_anomaly_hint_formatter.cpp
 */

#include "ai_anomaly_hint_formatter.hpp"

#include <sstream>

namespace scope::ai
{

std::string formatAiAnomalyHints(const std::vector<AiAnomalyHint>& hints)
{
    if (hints.empty())
    {
        return std::string();
    }

    std::ostringstream output;

    output << "Anomaly hints\n";

    for (const auto& hint : hints)
    {
        if (!hint.severity.empty())
        {
            output << "- [" << hint.severity << "] ";
        }
        else
        {
            output << "- ";
        }

        output << hint.message << "\n";
    }

    return output.str();
}

} // namespace scope::ai
