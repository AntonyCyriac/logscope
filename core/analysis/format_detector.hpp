/**
 * @file format_detector.hpp
 * @brief Detects log format from sampled line content.
 */

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "log_format.hpp"

namespace scope::analysis
{

/**
 * @brief Result of sampling leading log lines for format identification.
 */
struct FormatDetectionResult
{
    LogFormat format = LogFormat::Unknown;
    bool looksBinary = false;
    std::size_t sampledLines = 0U;
};

/**
 * @brief Identifies log format from a small sample of leading lines.
 */
class FormatDetector
{
  public:
    static constexpr std::size_t defaultSampleLineLimit = 32U;

    /**
     * @brief Detects format from already-read sample lines.
     *
     * @param sampleLines Leading lines from the source (without trailing newlines).
     * @return Detection result including binary and format classification.
     */
    [[nodiscard]] static FormatDetectionResult detect(const std::vector<std::string>& sampleLines);

    /**
     * @brief Detects format from a single contiguous text sample.
     */
    [[nodiscard]] static FormatDetectionResult detect(std::string_view sampleText);
};

} // namespace scope::analysis
