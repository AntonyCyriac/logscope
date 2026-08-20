/**
 * @file run_differ.cpp
 * @brief Signature diff implementation.
 */

#include "run_differ.hpp"

#include <algorithm>
#include <unordered_set>

namespace scope::compare
{

namespace
{

[[nodiscard]] bool signatureInMatchedScope(const SignatureEntry& entry, const AlignmentPlan& plan) noexcept
{
    if (plan.implicitSingleFilePair)
    {
        return true;
    }

    return std::find(plan.alignment.matchedInstances.begin(), plan.alignment.matchedInstances.end(),
                     entry.sampleLocation.instanceKey) != plan.alignment.matchedInstances.end();
}

[[nodiscard]] SignatureMap filterSignatures(const SignatureMap& source, const AlignmentPlan& plan)
{
    SignatureMap filtered;

    for (const auto& pair : source)
    {
        if (signatureInMatchedScope(pair.second, plan))
        {
            filtered.insert(pair);
        }
    }

    return filtered;
}

[[nodiscard]] std::vector<SignatureEntry> collectOnlyIn(const SignatureMap& left, const SignatureMap& right)
{
    std::vector<SignatureEntry> entries;

    for (const auto& pair : left)
    {
        if (right.find(pair.first) == right.end())
        {
            entries.push_back(pair.second);
        }
    }

    std::sort(entries.begin(),
              entries.end(),
              [](const SignatureEntry& leftEntry, const SignatureEntry& rightEntry) {
                  return leftEntry.signature < rightEntry.signature;
              });

    return entries;
}

[[nodiscard]] std::vector<CountDelta> collectCountDeltas(const SignatureMap& baseline,
                                                         const SignatureMap& candidate)
{
    std::vector<CountDelta> deltas;

    for (const auto& pair : baseline)
    {
        const auto candidateIt = candidate.find(pair.first);

        if (candidateIt == candidate.end())
        {
            continue;
        }

        if (pair.second.count == candidateIt->second.count)
        {
            continue;
        }

        CountDelta delta;
        delta.signature = pair.first;
        delta.baseline = pair.second.count;
        delta.candidate = candidateIt->second.count;
        deltas.push_back(std::move(delta));
    }

    std::sort(deltas.begin(),
              deltas.end(),
              [](const CountDelta& left, const CountDelta& right) { return left.signature < right.signature; });

    return deltas;
}

[[nodiscard]] SignatureMap signaturesForInstance(const SignatureMap& source, const std::string& instanceKey)
{
    SignatureMap filtered;

    for (const auto& pair : source)
    {
        if (pair.second.sampleLocation.instanceKey == instanceKey)
        {
            filtered.insert(pair);
        }
    }

    return filtered;
}

} // namespace

ComparisonResult diffAlignedRuns(const RunSnapshot& baseline, const RunSnapshot& candidate,
                                 const AlignmentPlan& plan)
{
    ComparisonResult result;
    result.comparable = true;
    result.alignment = plan.alignment;

    const SignatureMap baselineScoped = filterSignatures(baseline.signatures, plan);
    const SignatureMap candidateScoped = filterSignatures(candidate.signatures, plan);

    result.onlyInBaseline = collectOnlyIn(baselineScoped, candidateScoped);
    result.onlyInCandidate = collectOnlyIn(candidateScoped, baselineScoped);
    result.countDeltas = collectCountDeltas(baselineScoped, candidateScoped);

    if (!baseline.analysis.complete || !candidate.analysis.complete)
    {
        result.complete = false;
        result.warnings.push_back(
            ComparisonWarning{"COMPARISON_BOUNDED",
                              "One or both runs reported incomplete analysis; comparison may be bounded."});
    }

    if (plan.alignment.matchedInstances.size() > 1U)
    {
        for (const std::string& instanceKey : plan.alignment.matchedInstances)
        {
            const SignatureMap baselineInstance = signaturesForInstance(baselineScoped, instanceKey);
            const SignatureMap candidateInstance = signaturesForInstance(candidateScoped, instanceKey);

            InstanceComparison instanceComparison;
            instanceComparison.instanceKey = instanceKey;
            instanceComparison.onlyInBaseline = collectOnlyIn(baselineInstance, candidateInstance);
            instanceComparison.onlyInCandidate = collectOnlyIn(candidateInstance, baselineInstance);
            instanceComparison.countDeltas = collectCountDeltas(baselineInstance, candidateInstance);
            result.perInstance.push_back(std::move(instanceComparison));
        }
    }

    return result;
}

} // namespace scope::compare
