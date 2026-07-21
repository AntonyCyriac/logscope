/**
 * @file analysis_engine.hpp
 * @brief Analyzes source datasets and produces analysis models.
 */

#pragma once

#include "analysis_model.hpp"
#include "foundation/result.hpp"
#include "log_format.hpp"
#include "source_dataset.hpp"

namespace scope::analysis
{

/**
 * @brief Orchestrates analysis of log source datasets.
 */
class AnalysisEngine
{
  public:
    /**
     * @brief Analyzes a source dataset.
     *
     * @param dataset Prepared source dataset to analyze.
     * @param formatHint Auto (detect), or an explicit plain/jsonl override.
     * @return Analysis model or error.
     */
    [[nodiscard]] foundation::Result<AnalysisModel> analyze(
        source::SourceDataset& dataset, LogFormat formatHint = LogFormat::Auto) const;
};

} // namespace scope::analysis
