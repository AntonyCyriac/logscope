/**
 * @file crash_report.cpp
 */

#include "crash_report.hpp"

#include "foundation/hash.hpp"

#include <iomanip>
#include <sstream>

namespace scope::workspace
{

namespace
{

std::string formatHashHex(const std::uint64_t value)
{
    std::ostringstream output;
    output << std::hex << std::setw(16) << std::setfill('0') << value;

    return output.str();
}

} // namespace

std::string crashAnalysisStatusToString(const CrashAnalysisStatus status) noexcept
{
    switch (status)
    {
    case CrashAnalysisStatus::Ready:
        return "ready";
    case CrashAnalysisStatus::Unavailable:
        return "unavailable";
    case CrashAnalysisStatus::NotSupported:
        return "not_supported";
    case CrashAnalysisStatus::Failed:
        return "failed";
    }

    return "failed";
}

std::string makeCrashReportId(const std::string& investigationId, const std::string& artifactId,
                              const std::string& analyzerVersion)
{
    std::ostringstream key;
    key << investigationId << '|' << artifactId << '|' << analyzerVersion;

    return formatHashHex(foundation::hashString(key.str()));
}

} // namespace scope::workspace
