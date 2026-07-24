/**
 * @file indexed_line_access.hpp
 * @brief Unified indexed-line access for analysis consumers.
 */

#pragma once

#include <cstdint>
#include <vector>

#include "analysis_model.hpp"
#include "line_index.hpp"

namespace scope::analysis
{

[[nodiscard]] bool hasQueryableIndex(const AnalysisModel& model) noexcept;

[[nodiscard]] std::vector<IndexedLine> fetchIndexedLines(const AnalysisModel& model);

[[nodiscard]] std::uint64_t indexedLineCountForModel(const AnalysisModel& model) noexcept;

[[nodiscard]] std::uint64_t truncatedLineCountForModel(const AnalysisModel& model) noexcept;

} // namespace scope::analysis
