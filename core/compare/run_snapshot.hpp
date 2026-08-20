/**
 * @file run_snapshot.hpp
 * @brief Error signature snapshot from an analyzed run.
 */

#pragma once

#include <string>
#include <unordered_map>

#include "analysis_model.hpp"
#include "comparison_result.hpp"
#include "discovery_census.hpp"

namespace scope::compare
{

using SignatureMap = std::unordered_map<std::string, SignatureEntry>;

struct RunSnapshot
{
    source::DiscoveryCensus discovery;
    source::AnalysisAccounting analysis;
    SignatureMap signatures;
    bool rootIsFile{false};
};

[[nodiscard]] RunSnapshot buildRunSnapshot(const analysis::AnalysisModel& model);

} // namespace scope::compare
