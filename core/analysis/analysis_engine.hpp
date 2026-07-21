/**
 * @file analysis_engine.hpp
 * @brief Analyzes source datasets and produces analysis models.
 */

#pragma once

#include "analysis_model.hpp"
#include "analysis_config.hpp"
#include "foundation/result.hpp"
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
        source::SourceDataset& dataset, const AnalysisConfig& config = AnalysisConfig::defaults()) const;

    /**
     * @brief Analyzes a source dataset with a format hint only.
     */
    [[nodiscard]] foundation::Result<AnalysisModel> analyze(source::SourceDataset& dataset,
                                                            LogFormat formatHint) const;
};

} // namespace scope::analysis
