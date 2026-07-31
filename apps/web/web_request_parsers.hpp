/**
 * @file web_request_parsers.hpp
 * @brief Maps REST JSON bodies to domain configuration (M15.1).
 */

#pragma once

#include <string>

#include "analytics_config.hpp"
#include "analysis_config.hpp"
#include "application_service.hpp"
#include "configuration_manager.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "investigation_criteria.hpp"
#include "report_options.hpp"
#include "web_config.hpp"

namespace scope::web
{

[[nodiscard]] foundation::Result<foundation::Path> parsePathField(std::string_view body);

[[nodiscard]] investigation::InvestigationCriteria parseInvestigationCriteria(std::string_view body);

[[nodiscard]] analysis::AnalysisConfig parseAnalysisConfig(std::string_view body,
                                                             const configuration::ConfigurationManager& manager);

[[nodiscard]] analytics::AnalyticsConfig parseAnalyticsConfig(std::string_view body);

[[nodiscard]] reporting::ReportOptions parseReportOptions(std::string_view body);

[[nodiscard]] application::SessionSaveRequest parseSessionSaveRequest(std::string_view body);

/**
 * @brief Parsed body for POST /api/v1/agent/investigate.
 */
struct AgentInvestigateRequest
{
    investigation::InvestigationCriteria criteria;
    std::string askQuery;
    bool summarize = false;
    bool hints = false;
};

[[nodiscard]] AgentInvestigateRequest parseAgentInvestigateRequest(std::string_view body);

[[nodiscard]] foundation::Result<bool> validateServerPath(const WebConfig& config, const foundation::Path& path);

} // namespace scope::web
