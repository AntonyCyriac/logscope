/**
 * @file crash_report.hpp
 * @brief Crash analysis projection types (Story 4 / v2.6.0).
 */

#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "foundation/path.hpp"

namespace scope::workspace
{

/**
 * @brief A single stack frame in a crash report.
 */
struct CrashFrame
{
    std::size_t index = 0U;
    std::string address;
    std::string symbol;
    std::optional<std::string> module;
    std::optional<std::string> location;
};

/**
 * @brief A thread captured in a crash report.
 */
struct CrashThread
{
    std::string id;
    std::string name;
    std::vector<CrashFrame> frames;
    bool isFaultThread = false;
};

/**
 * @brief Outcome of crash analysis for an artifact.
 */
enum class CrashAnalysisStatus
{
    Ready,
    Unavailable,
    NotSupported,
    Failed
};

/**
 * @brief Derived crash evidence for one artifact (never persisted in manifest).
 */
struct CrashReport
{
    std::string id;
    std::string artifactId;
    std::string artifactType;
    CrashAnalysisStatus status = CrashAnalysisStatus::Unavailable;
    std::optional<std::string> signal;
    std::optional<std::string> faultThreadId;
    std::string summary;
    std::vector<CrashThread> threads;
    std::vector<std::string> observations;
    std::vector<std::string> warnings;
    std::map<std::string, std::string> metadata;
};

/**
 * @brief Context passed to crash analyzers.
 */
struct CrashAnalysisContext
{
    std::string investigationId;
    foundation::Path investigationRoot;
};

[[nodiscard]] std::string crashAnalysisStatusToString(CrashAnalysisStatus status) noexcept;

[[nodiscard]] std::string makeCrashReportId(const std::string& investigationId, const std::string& artifactId,
                                            const std::string& analyzerVersion);

} // namespace scope::workspace
