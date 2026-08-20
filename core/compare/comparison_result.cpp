/**
 * @file comparison_result.cpp
 * @brief Comparison result helpers.
 */

#include "comparison_result.hpp"

namespace scope::compare
{

const char* incomparableReasonName(const IncomparableReason reason) noexcept
{
    switch (reason)
    {
    case IncomparableReason::None:
        return "NONE";
    case IncomparableReason::BaselineIndeterminate:
        return "BASELINE_INDETERMINATE";
    case IncomparableReason::CandidateIndeterminate:
        return "CANDIDATE_INDETERMINATE";
    case IncomparableReason::BaselineUnsupported:
        return "BASELINE_UNSUPPORTED";
    case IncomparableReason::CandidateUnsupported:
        return "CANDIDATE_UNSUPPORTED";
    case IncomparableReason::NoInstanceOverlap:
        return "NO_INSTANCE_OVERLAP";
    case IncomparableReason::NoFileOverlap:
        return "NO_FILE_OVERLAP";
    }

    return "UNKNOWN";
}

} // namespace scope::compare
