/**
 * @file comparison_result.hpp
 * @brief Run comparison result types (ADR-014).
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace scope::compare
{

enum class IncomparableReason
{
    None,
    BaselineIndeterminate,
    CandidateIndeterminate,
    BaselineUnsupported,
    CandidateUnsupported,
    NoInstanceOverlap,
    NoFileOverlap
};

[[nodiscard]] const char* incomparableReasonName(IncomparableReason reason) noexcept;

struct SampleLocation
{
    std::string sourceFileRelative;
    std::uint64_t fileLineNumber{0U};
    std::string instanceKey;
};

struct SignatureEntry
{
    std::string signature;
    std::uint64_t count{0U};
    std::string sampleMessage;
    std::optional<std::string> firstSeen;
    SampleLocation sampleLocation;
};

struct CountDelta
{
    std::string signature;
    std::uint64_t baseline{0U};
    std::uint64_t candidate{0U};
};

struct AlignmentSummary
{
    std::vector<std::string> matchedInstances;
    std::vector<std::string> onlyInBaselineInstances;
    std::vector<std::string> onlyInCandidateInstances;
    std::vector<std::string> onlyInBaselineFiles;
    std::vector<std::string> onlyInCandidateFiles;
};

struct ComparisonWarning
{
    std::string code;
    std::string message;
};

struct InstanceComparison
{
    std::string instanceKey;
    std::vector<SignatureEntry> onlyInBaseline;
    std::vector<SignatureEntry> onlyInCandidate;
    std::vector<CountDelta> countDeltas;
};

struct ComparisonResult
{
    bool comparable{false};
    bool complete{true};
    std::optional<IncomparableReason> incomparableReason;
    AlignmentSummary alignment;
    std::vector<SignatureEntry> onlyInBaseline;
    std::vector<SignatureEntry> onlyInCandidate;
    std::vector<CountDelta> countDeltas;
    std::vector<InstanceComparison> perInstance;
    std::vector<ComparisonWarning> warnings;
};

} // namespace scope::compare
