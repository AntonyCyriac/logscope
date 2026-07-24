/**
 * @file report_writer.hpp
 * @brief Writes generated reports to files or streams.
 */

#pragma once

#include <iostream>
#include <optional>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "report.hpp"

namespace scope::cli
{

/**
 * @brief Writes a report to a file or stream.
 */
[[nodiscard]] foundation::Result<bool> writeReport(const reporting::Report& report,
                                                    const std::optional<foundation::Path>& outputFile,
                                                    std::ostream& output,
                                                    std::ostream& errorOutput);

} // namespace scope::cli
