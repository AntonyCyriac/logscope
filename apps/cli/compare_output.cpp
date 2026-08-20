/**
 * @file compare_output.cpp
 * @brief Compare output formatting.
 */

#include "compare_output.hpp"

#include <sstream>

namespace scope::cli
{

namespace
{

[[nodiscard]] std::string escapeJsonString(const std::string& value)
{
    std::ostringstream escaped;

    for (const char character : value)
    {
        switch (character)
        {
        case '"':
            escaped << "\\\"";
            break;
        case '\\':
            escaped << "\\\\";
            break;
        case '\n':
            escaped << "\\n";
            break;
        case '\r':
            escaped << "\\r";
            break;
        case '\t':
            escaped << "\\t";
            break;
        default:
            escaped << character;
            break;
        }
    }

    return escaped.str();
}

void writeSignatureArray(std::ostream& output, const std::vector<scope::compare::SignatureEntry>& entries)
{
    output << "[";

    for (std::size_t index = 0U; index < entries.size(); ++index)
    {
        const scope::compare::SignatureEntry& entry = entries[index];

        if (index > 0U)
        {
            output << ", ";
        }

        output << "{"
               << "\"signature\": \"" << escapeJsonString(entry.signature) << "\", "
               << "\"count\": " << entry.count << ", "
               << "\"sampleMessage\": \"" << escapeJsonString(entry.sampleMessage) << "\"";

        if (entry.firstSeen.has_value())
        {
            output << ", \"firstSeen\": \"" << escapeJsonString(*entry.firstSeen) << "\"";
        }

        output << ", \"sampleLocation\": {"
               << "\"sourceFile\": \"" << escapeJsonString(entry.sampleLocation.sourceFileRelative) << "\", "
               << "\"fileLineNumber\": " << entry.sampleLocation.fileLineNumber << ", "
               << "\"instanceKey\": \"" << escapeJsonString(entry.sampleLocation.instanceKey) << "\""
               << "}}";
    }

    output << "]";
}

void writeCountDeltas(std::ostream& output, const std::vector<scope::compare::CountDelta>& deltas)
{
    output << "[";

    for (std::size_t index = 0U; index < deltas.size(); ++index)
    {
        if (index > 0U)
        {
            output << ", ";
        }

        const scope::compare::CountDelta& delta = deltas[index];
        output << "{"
               << "\"signature\": \"" << escapeJsonString(delta.signature) << "\", "
               << "\"baseline\": " << delta.baseline << ", "
               << "\"candidate\": " << delta.candidate << "}";
    }

    output << "]";
}

void writeStringArray(std::ostream& output, const std::vector<std::string>& values)
{
    output << "[";

    for (std::size_t index = 0U; index < values.size(); ++index)
    {
        if (index > 0U)
        {
            output << ", ";
        }

        output << "\"" << escapeJsonString(values[index]) << "\"";
    }

    output << "]";
}

} // namespace

void writeCompareOutput(const scope::compare::ComparisonResult& result, const OutputFormat format, std::ostream& output)
{
    if (format == OutputFormat::Json)
    {
        output << "{\n  \"comparison\": {\n";
        output << "    \"comparable\": " << (result.comparable ? "true" : "false") << ",\n";
        output << "    \"complete\": " << (result.complete ? "true" : "false") << ",\n";

        if (result.incomparableReason.has_value())
        {
            output << "    \"incomparableReason\": \""
                   << scope::compare::incomparableReasonName(*result.incomparableReason) << "\",\n";
        }

        output << "    \"alignment\": {\n";
        output << "      \"matchedInstances\": ";
        writeStringArray(output, result.alignment.matchedInstances);
        output << ",\n      \"onlyInBaseline\": { \"instances\": ";
        writeStringArray(output, result.alignment.onlyInBaselineInstances);
        output << ", \"files\": ";
        writeStringArray(output, result.alignment.onlyInBaselineFiles);
        output << " },\n      \"onlyInCandidate\": { \"instances\": ";
        writeStringArray(output, result.alignment.onlyInCandidateInstances);
        output << ", \"files\": ";
        writeStringArray(output, result.alignment.onlyInCandidateFiles);
        output << " }\n    },\n";
        output << "    \"onlyInBaseline\": ";
        writeSignatureArray(output, result.onlyInBaseline);
        output << ",\n    \"onlyInCandidate\": ";
        writeSignatureArray(output, result.onlyInCandidate);
        output << ",\n    \"countDeltas\": ";
        writeCountDeltas(output, result.countDeltas);
        output << ",\n    \"warnings\": [";

        for (std::size_t index = 0U; index < result.warnings.size(); ++index)
        {
            if (index > 0U)
            {
                output << ", ";
            }

            output << "{"
                   << "\"code\": \"" << escapeJsonString(result.warnings[index].code) << "\", "
                   << "\"message\": \"" << escapeJsonString(result.warnings[index].message) << "\"}";
        }

        output << "]\n  }\n}\n";

        return;
    }

    output << "Run comparison\n";
    output << "Comparable : " << (result.comparable ? "yes" : "no") << '\n';
    output << "Complete   : " << (result.complete ? "yes" : "no") << '\n';

    if (result.incomparableReason.has_value())
    {
        output << "Reason     : " << scope::compare::incomparableReasonName(*result.incomparableReason) << '\n';
    }

    if (!result.onlyInBaseline.empty())
    {
        output << "\nOnly in baseline:\n";

        for (const scope::compare::SignatureEntry& entry : result.onlyInBaseline)
        {
            output << "  " << entry.signature << " (" << entry.count << ")\n";
        }
    }

    if (!result.onlyInCandidate.empty())
    {
        output << "\nOnly in candidate:\n";

        for (const scope::compare::SignatureEntry& entry : result.onlyInCandidate)
        {
            output << "  " << entry.signature << " (" << entry.count << ")\n";
        }
    }

    if (!result.countDeltas.empty())
    {
        output << "\nCount deltas:\n";

        for (const scope::compare::CountDelta& delta : result.countDeltas)
        {
            output << "  " << delta.signature << " (" << delta.baseline << " -> " << delta.candidate << ")\n";
        }
    }
}

} // namespace scope::cli
