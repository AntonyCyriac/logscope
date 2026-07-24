/**
 * @file builtin_extensions.cpp
 * @brief Registers built-in LogScope extensions.
 */

#include "extension_manager.hpp"
#include "report_section_renderer.hpp"

namespace scope::extension
{

namespace
{

foundation::Result<bool> initializeReportingMultiFormat()
{
    reporting::registerReportingContributors(reporting::ReportSectionRegistry::instance());

    return foundation::Result<bool>(true);
}

} // namespace

void registerBuiltInExtensions(ExtensionManager& manager)
{
    manager.registerBuiltIn(ExtensionDescriptor{
        "analysis.log-levels",
        "1.0.0",
        "Generic INFO, WARN, and ERROR line statistics during analysis.",
        nullptr,
        true});

    manager.registerBuiltIn(ExtensionDescriptor{
        "source.files",
        "1.0.0",
        "Single-file, directory, and stdin log source acquisition.",
        nullptr,
        true});

    manager.registerBuiltIn(ExtensionDescriptor{
        "reporting.multi-format",
        "1.3.0",
        "Text, JSON, CSV, Markdown, HTML, and PDF report output with selectable sections.",
        initializeReportingMultiFormat,
        true});
}

} // namespace scope::extension
