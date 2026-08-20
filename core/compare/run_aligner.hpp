/**
 * @file run_aligner.hpp
 * @brief Aligns baseline and candidate runs for comparison.
 */

#pragma once

#include "run_snapshot.hpp"

namespace scope::compare
{

struct AlignmentPlan
{
    bool comparable{false};
    std::optional<IncomparableReason> incomparableReason;
    AlignmentSummary alignment;
    bool implicitSingleFilePair{false};
};

[[nodiscard]] AlignmentPlan alignRuns(const RunSnapshot& baseline, const RunSnapshot& candidate);

} // namespace scope::compare
