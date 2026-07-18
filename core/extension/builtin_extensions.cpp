/**
 * @file builtin_extensions.cpp
 * @brief Registers built-in LogScope extensions.
 */

#include "extension_manager.hpp"

namespace scope::extension
{

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
        "1.0.0",
        "Text, JSON, CSV, and Markdown report output with selectable sections.",
        nullptr,
        true});
}

} // namespace scope::extension
