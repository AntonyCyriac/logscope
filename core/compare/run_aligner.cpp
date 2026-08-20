/**
 * @file run_aligner.cpp
 * @brief Run alignment implementation.
 */

#include "run_aligner.hpp"

#include <algorithm>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace scope::compare
{

namespace
{

[[nodiscard]] std::unordered_set<std::string> instanceKeysFromCensus(const source::DiscoveryCensus& census)
{
    std::unordered_set<std::string> keys;

    for (const source::InstanceSummary& instance : census.instances)
    {
        keys.insert(instance.key);
    }

    if (keys.empty())
    {
        for (const source::DiscoveryEntry& entry : census.entries)
        {
            if (entry.disposition == source::CandidateDisposition::Analyzed)
            {
                keys.insert(entry.instanceKey);
            }
        }
    }

    if (keys.empty())
    {
        keys.insert("default");
    }

    return keys;
}

[[nodiscard]] std::string logicalFileKey(const source::DiscoveryEntry& entry)
{
    if (entry.rotationGroupId.has_value())
    {
        return "rotation:" + *entry.rotationGroupId;
    }

    return "file:" + entry.relativePath;
}

[[nodiscard]] std::unordered_map<std::string, std::set<std::string>> logicalFilesByInstance(
    const source::DiscoveryCensus& census)
{
    std::unordered_map<std::string, std::set<std::string>> filesByInstance;

    for (const source::DiscoveryEntry& entry : census.entries)
    {
        if (entry.disposition != source::CandidateDisposition::Analyzed)
        {
            continue;
        }

        filesByInstance[entry.instanceKey].insert(logicalFileKey(entry));
    }

    return filesByInstance;
}

[[nodiscard]] std::uint32_t analyzedEntryCount(const source::DiscoveryCensus& census) noexcept
{
    std::uint32_t count = 0U;

    for (const source::DiscoveryEntry& entry : census.entries)
    {
        if (entry.disposition == source::CandidateDisposition::Analyzed)
        {
            ++count;
        }
    }

    return count;
}

} // namespace

AlignmentPlan alignRuns(const RunSnapshot& baseline, const RunSnapshot& candidate)
{
    AlignmentPlan plan;

    const std::unordered_set<std::string> baselineInstances = instanceKeysFromCensus(baseline.discovery);
    const std::unordered_set<std::string> candidateInstances = instanceKeysFromCensus(candidate.discovery);

    for (const std::string& key : baselineInstances)
    {
        if (candidateInstances.count(key) != 0U)
        {
            plan.alignment.matchedInstances.push_back(key);
        }
        else
        {
            plan.alignment.onlyInBaselineInstances.push_back(key);
        }
    }

    for (const std::string& key : candidateInstances)
    {
        if (baselineInstances.count(key) == 0U)
        {
            plan.alignment.onlyInCandidateInstances.push_back(key);
        }
    }

    std::sort(plan.alignment.matchedInstances.begin(), plan.alignment.matchedInstances.end());
    std::sort(plan.alignment.onlyInBaselineInstances.begin(), plan.alignment.onlyInBaselineInstances.end());
    std::sort(plan.alignment.onlyInCandidateInstances.begin(), plan.alignment.onlyInCandidateInstances.end());

    plan.implicitSingleFilePair =
        baseline.rootIsFile && candidate.rootIsFile && analyzedEntryCount(baseline.discovery) == 1U &&
        analyzedEntryCount(candidate.discovery) == 1U;

    if (plan.implicitSingleFilePair)
    {
        if (plan.alignment.matchedInstances.empty())
        {
            plan.alignment.matchedInstances.push_back("default");
        }

        plan.comparable = true;

        return plan;
    }

    if (plan.alignment.matchedInstances.empty())
    {
        plan.comparable = false;
        plan.incomparableReason = IncomparableReason::NoInstanceOverlap;

        return plan;
    }

    const auto baselineFiles = logicalFilesByInstance(baseline.discovery);
    const auto candidateFiles = logicalFilesByInstance(candidate.discovery);

    bool hasMatchedFile = false;

    for (const std::string& instanceKey : plan.alignment.matchedInstances)
    {
        const auto baselineIt = baselineFiles.find(instanceKey);
        const auto candidateIt = candidateFiles.find(instanceKey);

        if (baselineIt == baselineFiles.end() || candidateIt == candidateFiles.end())
        {
            continue;
        }

        for (const std::string& fileKey : baselineIt->second)
        {
            if (candidateIt->second.count(fileKey) != 0U)
            {
                hasMatchedFile = true;
            }
            else
            {
                plan.alignment.onlyInBaselineFiles.push_back(instanceKey + "/" + fileKey);
            }
        }

        for (const std::string& fileKey : candidateIt->second)
        {
            if (baselineIt->second.count(fileKey) == 0U)
            {
                plan.alignment.onlyInCandidateFiles.push_back(instanceKey + "/" + fileKey);
            }
        }
    }

    std::sort(plan.alignment.onlyInBaselineFiles.begin(), plan.alignment.onlyInBaselineFiles.end());
    std::sort(plan.alignment.onlyInCandidateFiles.begin(), plan.alignment.onlyInCandidateFiles.end());

    if (!hasMatchedFile)
    {
        plan.comparable = false;
        plan.incomparableReason = IncomparableReason::NoFileOverlap;

        return plan;
    }

    plan.comparable = true;

    return plan;
}

} // namespace scope::compare
