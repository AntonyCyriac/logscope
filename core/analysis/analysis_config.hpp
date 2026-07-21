/**
 * @file analysis_config.hpp
 * @brief Analysis behaviour resolved from profiles and configuration.
 */

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "foundation/result.hpp"
#include "json_field_mapping.hpp"
#include "line_index.hpp"
#include "log_format.hpp"
#include "runtime/configuration.hpp"

namespace scope::analysis
{

/// Maximum allowed configured index size.
constexpr std::size_t maxConfigurableIndexedLines = 1000000U;

/// Default investigation index capacity (from line_index.hpp).
constexpr std::size_t defaultIndexedLineCapacity = maxIndexedLines;

/**
 * @brief Resolved analysis settings for a single run.
 */
struct AnalysisConfig
{
    LogFormat formatHint{LogFormat::Auto};
    JsonFieldMapping jsonFieldMapping;
    std::size_t maxIndexedLines{defaultIndexedLineCapacity};

    [[nodiscard]] static AnalysisConfig defaults() noexcept;
};

/**
 * @brief Built-in named format profile.
 */
struct FormatProfile
{
    std::string name;
    AnalysisConfig defaults;
};

/**
 * @brief Resolves a built-in format profile by name.
 */
[[nodiscard]] std::optional<FormatProfile> resolveFormatProfile(std::string_view name) noexcept;

/**
 * @brief Lists supported built-in profile names.
 */
[[nodiscard]] std::string supportedProfilesList() noexcept;

/**
 * @brief Resolves analysis settings from configuration and CLI overrides.
 */
[[nodiscard]] AnalysisConfig resolveAnalysisConfig(const runtime::Configuration& configuration,
                                                   const AnalysisConfig& cliOverrides) noexcept;

/**
 * @brief Validates format-related configuration keys and values.
 */
[[nodiscard]] foundation::Result<bool> validateAnalysisConfiguration(
    const runtime::Configuration& configuration) noexcept;

} // namespace scope::analysis
