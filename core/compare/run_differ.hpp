/**
 * @file run_differ.hpp
 * @brief Signature diff between aligned runs.
 */

#pragma once

#include "comparison_result.hpp"
#include "run_aligner.hpp"
#include "run_snapshot.hpp"

namespace scope::compare
{

[[nodiscard]] ComparisonResult diffAlignedRuns(const RunSnapshot& baseline, const RunSnapshot& candidate,
                                             const AlignmentPlan& plan);

} // namespace scope::compare
