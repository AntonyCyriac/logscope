/**
 * @file run_comparison.hpp
 * @brief Orchestrates run comparison (ADR-014).
 */

#pragma once

#include "analysis_model.hpp"
#include "comparison_result.hpp"
#include "foundation/error.hpp"

namespace scope::compare
{

enum class RunLoadFailure
{
    None,
    Indeterminate,
    Unsupported,
    Other
};

struct LoadedRun
{
    analysis::AnalysisModel model;
    RunLoadFailure failure{RunLoadFailure::None};
};

[[nodiscard]] ComparisonResult compareModels(const analysis::AnalysisModel& baseline,
                                             const analysis::AnalysisModel& candidate);

[[nodiscard]] ComparisonResult incomparableResult(IncomparableReason reason);

} // namespace scope::compare
