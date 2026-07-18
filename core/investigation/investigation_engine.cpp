/**
 * @file investigation_engine.cpp
 * @brief InvestigationEngine implementation.
 */

#include "investigation_engine.hpp"

#include "foundation/string.hpp"
#include "log_macros.hpp"

namespace scope::investigation
{

InvestigationView InvestigationEngine::inspect(const analysis::AnalysisModel& model) const
{
    SCOPE_LOG_INFO("investigation", "Inspecting analysis for " + model.sourcePath().string());

    return InvestigationView(model.sourcePath(), model.totalLines());
}

bool InvestigationEngine::matches(const analysis::AnalysisModel& model,
                                  const LineCountFilter& filter) const noexcept
{
    return filter.matches(model);
}

bool InvestigationEngine::searchSource(const analysis::AnalysisModel& model,
                                       const std::string_view query) const noexcept
{
    if (foundation::isBlank(query))
    {
        return true;
    }

    const std::string sourcePath = foundation::toLower(model.sourcePath().string());
    const std::string loweredQuery = foundation::toLower(query);

    return sourcePath.find(loweredQuery) != std::string::npos;
}

} // namespace scope::investigation
