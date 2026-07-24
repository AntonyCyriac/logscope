/**
 * @file analytics_output.hpp
 * @brief Serializes analytics results for CLI output.
 */

#pragma once

#include <iosfwd>

#include "analytics_result.hpp"
#include "output_format.hpp"

namespace scope::cli
{

void writeAnalyticsOutput(const analytics::AnalyticsResult& result,
                          OutputFormat format,
                          std::ostream& output);

} // namespace scope::cli
