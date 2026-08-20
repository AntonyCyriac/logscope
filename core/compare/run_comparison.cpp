/**
 * @file run_comparison.cpp
 * @brief Run comparison orchestration.
 */

#include "run_comparison.hpp"

#include "run_aligner.hpp"
#include "run_differ.hpp"
#include "run_snapshot.hpp"

namespace scope::compare
{

ComparisonResult incomparableResult(const IncomparableReason reason)
{
    ComparisonResult result;
    result.comparable = false;
    result.incomparableReason = reason;

    return result;
}

ComparisonResult compareModels(const analysis::AnalysisModel& baseline, const analysis::AnalysisModel& candidate)
{
    const RunSnapshot baselineSnapshot = buildRunSnapshot(baseline);
    const RunSnapshot candidateSnapshot = buildRunSnapshot(candidate);

    if (baselineSnapshot.analysis.analyzedLineCount == 0U && baselineSnapshot.discovery.candidatesFound > 0U)
    {
        return incomparableResult(IncomparableReason::BaselineIndeterminate);
    }

    if (candidateSnapshot.analysis.analyzedLineCount == 0U && candidateSnapshot.discovery.candidatesFound > 0U)
    {
        return incomparableResult(IncomparableReason::CandidateIndeterminate);
    }

    const AlignmentPlan plan = alignRuns(baselineSnapshot, candidateSnapshot);

    if (!plan.comparable)
    {
        ComparisonResult result;
        result.comparable = false;
        result.incomparableReason = plan.incomparableReason;
        result.alignment = plan.alignment;

        return result;
    }

    return diffAlignedRuns(baselineSnapshot, candidateSnapshot, plan);
}

} // namespace scope::compare
