/**
 * @file compare_output.hpp
 * @brief Compare command output formatting.
 */

#pragma once

#include <iosfwd>

#include "compare.hpp"
#include "output_format.hpp"

namespace scope::cli
{

void writeCompareOutput(const scope::compare::ComparisonResult& result, OutputFormat format, std::ostream& output);

} // namespace scope::cli
