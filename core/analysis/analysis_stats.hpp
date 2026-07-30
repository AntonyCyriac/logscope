/**
 * @file analysis_stats.hpp
 * @brief Timing and volume metrics for log analysis (Phase 1 observability).
 */

#pragma once

#include <cstdint>

#include "foundation/duration.hpp"

namespace scope::analysis
{

/**
 * @brief Parse timing and volume metrics collected during analysis.
 */
struct AnalysisStats
{
    foundation::Duration parseDuration{};
    std::uint64_t lineCount{0U};
    std::uint64_t byteCount{0U};
    bool indexReused{false};
};

} // namespace scope::analysis
