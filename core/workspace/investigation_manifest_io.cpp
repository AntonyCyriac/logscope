/**
 * @file investigation_manifest_io.cpp
 * @brief Read/write investigation manifest.json.
 */

#include "investigation_manifest_io.hpp"

#include "foundation/clock.hpp"
#include "foundation/uuid.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

namespace scope::workspace
{

namespace
{

constexpr const char* manifestFileName = "manifest.json";
constexpr const char* legacyWorkspaceFileName = "workspace.json";

std::string_view skipWhitespace(std::string_view value)
{
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0)
    {
        value.remove_prefix(1U);
    }

    return value;
}

std::optional<std::string> jsonStringField(const std::string_view body, const std::string_view key)
{
    const std::size_t keyPosition = body.find('"' + std::string(key) + '"');

    if (keyPosition == std::string::npos)
    {
        return std::nullopt;
    }

    std::string_view remainder = body.substr(keyPosition);
    const std::size_t colon = remainder.find(':');

    if (colon == std::string::npos)
    {
        return std::nullopt;
    }

    remainder = skipWhitespace(remainder.substr(colon + 1U));

    if (remainder.empty() || remainder.front() != '"')
    {
        return std::nullopt;
    }

    remainder.remove_prefix(1U);

    std::string value;

    for (std::size_t index = 0U; index < remainder.size(); ++index)
    {
        const char character = remainder[index];

        if (character == '\\' && index + 1U < remainder.size())
        {
            value.push_back(remainder[index + 1U]);
            ++index;

            continue;
        }

        if (character == '"')
        {
            return value;
        }

        value.push_back(character);
    }

    return std::nullopt;
}

std::string extractJsonString(const std::string& object, const std::string_view key)
{
    const std::optional<std::string> value = jsonStringField(object, key);

    return value.value_or(std::string());
}

std::optional<bool> jsonBoolField(const std::string_view body, const std::string_view key)
{
    const std::size_t keyPosition = body.find('"' + std::string(key) + '"');

    if (keyPosition == std::string::npos)
    {
        return std::nullopt;
    }

    std::string_view remainder = body.substr(keyPosition);
    const std::size_t colon = remainder.find(':');

    if (colon == std::string::npos)
    {
        return std::nullopt;
    }

    remainder = skipWhitespace(remainder.substr(colon + 1U));

    if (remainder.rfind("true", 0) == 0)
    {
        return true;
    }

    if (remainder.rfind("false", 0) == 0)
    {
        return false;
    }

    return std::nullopt;
}

std::optional<std::int64_t> jsonIntField(const std::string_view body, const std::string_view key)
{
    const std::size_t keyPosition = body.find('"' + std::string(key) + '"');

    if (keyPosition == std::string::npos)
    {
        return std::nullopt;
    }

    std::string_view remainder = body.substr(keyPosition);
    const std::size_t colon = remainder.find(':');

    if (colon == std::string::npos)
    {
        return std::nullopt;
    }

    remainder = skipWhitespace(remainder.substr(colon + 1U));

    std::int64_t value = 0;
    bool negative = false;

    if (!remainder.empty() && remainder.front() == '-')
    {
        negative = true;
        remainder.remove_prefix(1U);
    }

    if (remainder.empty() || !std::isdigit(static_cast<unsigned char>(remainder.front())))
    {
        return std::nullopt;
    }

    for (const char character : remainder)
    {
        if (!std::isdigit(static_cast<unsigned char>(character)))
        {
            break;
        }

        value = value * 10 + static_cast<std::int64_t>(character - '0');
    }

    return negative ? -value : value;
}

std::optional<std::size_t> findJsonValueStart(const std::string& body, const std::string_view key)
{
    const std::string needle = '"' + std::string(key) + '"';
    std::size_t searchFrom = 0U;

    while (searchFrom < body.size())
    {
        const std::size_t keyPosition = body.find(needle, searchFrom);

        if (keyPosition == std::string::npos)
        {
            return std::nullopt;
        }

        std::size_t index = keyPosition + needle.size();

        while (index < body.size() && std::isspace(static_cast<unsigned char>(body[index])) != 0)
        {
            ++index;
        }

        if (index < body.size() && body[index] == ':')
        {
            ++index;

            while (index < body.size() && std::isspace(static_cast<unsigned char>(body[index])) != 0)
            {
                ++index;
            }

            return index;
        }

        searchFrom = keyPosition + 1U;
    }

    return std::nullopt;
}

std::optional<std::string> extractJsonObject(const std::string& body, const std::string_view key)
{
    const std::optional<std::size_t> valueStart = findJsonValueStart(body, key);

    if (!valueStart)
    {
        return std::nullopt;
    }

    const std::size_t openBrace = body.find('{', *valueStart);

    if (openBrace == std::string::npos)
    {
        return std::nullopt;
    }

    std::size_t depth = 0U;
    std::size_t closeBrace = openBrace;

    for (std::size_t index = openBrace; index < body.size(); ++index)
    {
        if (body[index] == '{')
        {
            ++depth;
        }
        else if (body[index] == '}')
        {
            --depth;

            if (depth == 0U)
            {
                closeBrace = index;
                break;
            }
        }
    }

    return body.substr(openBrace, closeBrace - openBrace + 1U);
}

std::vector<std::string> extractJsonStringArray(const std::string& body, const std::string_view key)
{
    std::vector<std::string> values;
    const std::optional<std::size_t> valueStart = findJsonValueStart(body, key);

    if (!valueStart)
    {
        return values;
    }

    const std::size_t openBracket = body.find('[', *valueStart);

    if (openBracket == std::string::npos)
    {
        return values;
    }

    std::size_t depth = 0U;
    std::size_t closeBracket = openBracket;

    for (std::size_t index = openBracket; index < body.size(); ++index)
    {
        if (body[index] == '[')
        {
            ++depth;
        }
        else if (body[index] == ']')
        {
            --depth;

            if (depth == 0U)
            {
                closeBracket = index;
                break;
            }
        }
    }

    const std::string arrayBody = body.substr(openBracket, closeBracket - openBracket + 1U);
    std::size_t index = 0U;

    while (index < arrayBody.size())
    {
        const std::size_t quote = arrayBody.find('"', index);

        if (quote == std::string::npos)
        {
            break;
        }

        std::size_t cursor = quote + 1U;
        std::string value;

        while (cursor < arrayBody.size())
        {
            const char character = arrayBody[cursor];

            if (character == '\\' && cursor + 1U < arrayBody.size())
            {
                value.push_back(arrayBody[cursor + 1U]);
                cursor += 2U;

                continue;
            }

            if (character == '"')
            {
                values.push_back(std::move(value));
                index = cursor + 1U;

                break;
            }

            value.push_back(character);
            ++cursor;
        }

        if (cursor >= arrayBody.size())
        {
            break;
        }
    }

    return values;
}

std::vector<std::string> extractJsonObjectArray(const std::string& body, const std::string_view key)
{
    std::vector<std::string> objects;
    const std::optional<std::size_t> valueStart = findJsonValueStart(body, key);

    if (!valueStart)
    {
        return objects;
    }

    const std::size_t openBracket = body.find('[', *valueStart);

    if (openBracket == std::string::npos)
    {
        return objects;
    }

    std::size_t index = openBracket + 1U;

    while (index < body.size())
    {
        index = body.find_first_not_of(" \t\r\n,", index);

        if (index == std::string::npos || body[index] == ']')
        {
            break;
        }

        if (body[index] != '{')
        {
            break;
        }

        std::size_t depth = 0U;
        std::size_t closeBrace = index;

        for (std::size_t cursor = index; cursor < body.size(); ++cursor)
        {
            if (body[cursor] == '{')
            {
                ++depth;
            }
            else if (body[cursor] == '}')
            {
                --depth;

                if (depth == 0U)
                {
                    closeBrace = cursor;
                    break;
                }
            }
        }

        objects.push_back(body.substr(index, closeBrace - index + 1U));
        index = closeBrace + 1U;
    }

    return objects;
}

ArtifactSource parseArtifactSource(const std::string& object)
{
    ArtifactSource source;
    source.kind = extractJsonString(object, "kind");

    if (source.kind.empty())
    {
        source.kind = "upload";
    }

    source.displayName = extractJsonString(object, "displayName");

    return source;
}

InvestigationSummary parseSummary(const std::string& body)
{
    InvestigationSummary summary;
    const std::optional<std::string> summaryObject = extractJsonObject(body, "summary");

    if (!summaryObject)
    {
        return summary;
    }

    if (const std::optional<bool> hasModel = jsonBoolField(*summaryObject, "hasModel"))
    {
        summary.hasModel = *hasModel;
    }

    if (const std::optional<std::int64_t> lineCount = jsonIntField(*summaryObject, "lineCount"))
    {
        summary.lineCount = static_cast<std::uint64_t>(*lineCount);
    }

    if (const std::optional<std::int64_t> errorCount = jsonIntField(*summaryObject, "errorCount"))
    {
        summary.errorCount = static_cast<std::uint64_t>(*errorCount);
    }

    return summary;
}

ArtifactRecord parseArtifactRecord(const std::string& object)
{
    ArtifactRecord record;
    record.id = extractJsonString(object, "id");
    record.type = extractJsonString(object, "type");
    record.name = extractJsonString(object, "name");
    record.relativePath = extractJsonString(object, "relativePath");
    record.importedAt = extractJsonString(object, "importedAt");
    record.status = extractJsonString(object, "status");

    if (record.status.empty())
    {
        record.status = "ready";
    }

    const std::optional<std::string> sourceObject = extractJsonObject(object, "source");

    if (sourceObject)
    {
        record.source = parseArtifactSource(*sourceObject);
    }

    record.tags = extractJsonStringArray(object, "tags");

    return record;
}

std::string escapeJsonString(const std::string_view value)
{
    std::ostringstream output;

    for (const char character : value)
    {
        switch (character)
        {
        case '"':
            output << "\\\"";
            break;
        case '\\':
            output << "\\\\";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        default:
            output << character;
            break;
        }
    }

    return output.str();
}

std::string formatStringArrayJson(const std::vector<std::string>& values)
{
    std::ostringstream output;
    output << '[';

    for (std::size_t index = 0U; index < values.size(); ++index)
    {
        if (index > 0U)
        {
            output << ", ";
        }

        output << '"' << escapeJsonString(values[index]) << '"';
    }

    output << ']';

    return output.str();
}

std::string formatArtifactSourceJson(const ArtifactSource& source)
{
    std::ostringstream output;
    output << "{\n"
           << "        \"kind\": \"" << escapeJsonString(source.kind) << "\",\n"
           << "        \"displayName\": \"" << escapeJsonString(source.displayName) << "\"\n"
           << "      }";

    return output.str();
}

std::string formatArtifactRecordJson(const ArtifactRecord& record)
{
    std::ostringstream output;
    output << "    {\n"
           << "      \"id\": \"" << escapeJsonString(record.id) << "\",\n"
           << "      \"type\": \"" << escapeJsonString(record.type) << "\",\n"
           << "      \"name\": \"" << escapeJsonString(record.name) << "\",\n"
           << "      \"relativePath\": \"" << escapeJsonString(record.relativePath) << "\",\n"
           << "      \"importedAt\": \"" << escapeJsonString(record.importedAt) << "\",\n"
           << "      \"source\": " << formatArtifactSourceJson(record.source) << ",\n"
           << "      \"status\": \"" << escapeJsonString(record.status) << "\",\n"
           << "      \"tags\": " << formatStringArrayJson(record.tags) << ",\n"
           << "      \"metadata\": {}\n"
           << "    }";

    return output.str();
}

std::string formatSummaryJson(const InvestigationSummary& summary)
{
    std::ostringstream output;
    output << "{\n"
           << "    \"hasModel\": " << (summary.hasModel ? "true" : "false") << ",\n"
           << "    \"lineCount\": " << summary.lineCount << ",\n"
           << "    \"errorCount\": " << summary.errorCount << "\n"
           << "  }";

    return output.str();
}

std::string formatManifestJson(const InvestigationManifest& manifest)
{
    std::ostringstream output;
    output << "{\n"
           << "  \"schemaVersion\": " << manifest.schemaVersion << ",\n"
           << "  \"id\": \"" << escapeJsonString(manifest.id) << "\",\n"
           << "  \"name\": \"" << escapeJsonString(manifest.name) << "\",\n"
           << "  \"description\": \"" << escapeJsonString(manifest.description) << "\",\n"
           << "  \"createdAt\": \"" << escapeJsonString(manifest.createdAt) << "\",\n"
           << "  \"updatedAt\": \"" << escapeJsonString(manifest.updatedAt) << "\",\n"
           << "  \"primaryArtifactId\": \"" << escapeJsonString(manifest.primaryArtifactId) << "\",\n"
           << "  \"tags\": " << formatStringArrayJson(manifest.tags) << ",\n"
           << "  \"metadata\": {},\n"
           << "  \"artifacts\": [\n";

    for (std::size_t index = 0U; index < manifest.artifacts.size(); ++index)
    {
        if (index > 0U)
        {
            output << ",\n";
        }

        output << formatArtifactRecordJson(manifest.artifacts[index]);
    }

    output << "\n  ],\n"
           << "  \"summary\": " << formatSummaryJson(manifest.summary) << ",\n"
           << "  \"snapshotFile\": \"" << escapeJsonString(manifest.snapshotFile) << "\"\n"
           << '}';

    return output.str();
}

foundation::Result<std::string> readFileContent(const foundation::Path& path)
{
    std::ifstream stream(path.string());

    if (!stream)
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "File not found."));
    }

    std::ostringstream buffer;
    buffer << stream.rdbuf();

    return foundation::Result<std::string>(buffer.str());
}

InvestigationManifest parseManifestContent(const std::string& content)
{
    InvestigationManifest manifest;

    if (const std::optional<std::int64_t> schemaVersion = jsonIntField(content, "schemaVersion"))
    {
        manifest.schemaVersion = static_cast<int>(*schemaVersion);
    }

    manifest.id = extractJsonString(content, "id");
    manifest.name = extractJsonString(content, "name");
    manifest.description = extractJsonString(content, "description");
    manifest.createdAt = extractJsonString(content, "createdAt");
    manifest.updatedAt = extractJsonString(content, "updatedAt");
    manifest.primaryArtifactId = extractJsonString(content, "primaryArtifactId");
    manifest.snapshotFile = extractJsonString(content, "snapshotFile");

    if (manifest.snapshotFile.empty())
    {
        manifest.snapshotFile = "snapshot.session";
    }

    manifest.tags = extractJsonStringArray(content, "tags");
    manifest.summary = parseSummary(content);

    const std::vector<std::string> artifactObjects = extractJsonObjectArray(content, "artifacts");

    for (const std::string& artifactObject : artifactObjects)
    {
        manifest.artifacts.push_back(parseArtifactRecord(artifactObject));
    }

    return manifest;
}

} // namespace

foundation::Result<InvestigationManifest> loadManifest(const foundation::Path& investigationDir)
{
    const foundation::Path manifestPath =
        foundation::Path(investigationDir.string() + "/" + manifestFileName);
    const auto contentResult = readFileContent(manifestPath);

    if (!contentResult)
    {
        return foundation::Result<InvestigationManifest>(contentResult.error());
    }

    return foundation::Result<InvestigationManifest>(parseManifestContent(*contentResult));
}

foundation::Result<bool> saveManifest(const foundation::Path& investigationDir,
                                      const InvestigationManifest& manifest)
{
    const foundation::Path manifestPath =
        foundation::Path(investigationDir.string() + "/" + manifestFileName);
    const foundation::Path tempPath =
        foundation::Path(investigationDir.string() + "/" + manifestFileName + ".tmp");
    const std::string json = formatManifestJson(manifest);

    {
        std::ofstream stream(tempPath.string(), std::ios::binary | std::ios::trunc);

        if (!stream)
        {
            return foundation::Result<bool>(
                foundation::Error(foundation::ErrorCode::IOError, "Failed to write investigation manifest."));
        }

        stream << json;
    }

    std::error_code errorCode;
    std::filesystem::rename(tempPath.string(), manifestPath.string(), errorCode);

    if (errorCode)
    {
        return foundation::Result<bool>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to finalize investigation manifest."));
    }

    return foundation::Result<bool>(true);
}

foundation::Result<InvestigationManifest> migrateLegacyWorkspaceMetadata(
    const foundation::Path& investigationDir, const std::string& workspaceJson)
{
    InvestigationManifest manifest;
    manifest.schemaVersion = 1;
    manifest.id = extractJsonString(workspaceJson, "id");
    manifest.name = extractJsonString(workspaceJson, "name");
    manifest.description = extractJsonString(workspaceJson, "description");
    manifest.createdAt = extractJsonString(workspaceJson, "createdAt");
    manifest.updatedAt = extractJsonString(workspaceJson, "updatedAt");
    manifest.snapshotFile = extractJsonString(workspaceJson, "snapshotFile");

    if (manifest.snapshotFile.empty())
    {
        manifest.snapshotFile = "snapshot.session";
    }

    manifest.summary = parseSummary(workspaceJson);

    const std::string directoryName = std::filesystem::path(investigationDir.string()).filename().string();

    if (manifest.id.empty())
    {
        manifest.id = directoryName;
    }

    ArtifactRecord artifact;
    artifact.id = foundation::Uuid::generate().toString();
    artifact.type = "log";
    artifact.importedAt = manifest.createdAt.empty() ? foundation::Clock::now().toString() : manifest.createdAt;
    artifact.status = "ready";

    const std::optional<std::string> sourceRefObject = extractJsonObject(workspaceJson, "sourceRef");

    if (sourceRefObject)
    {
        artifact.source.kind = extractJsonString(*sourceRefObject, "type");

        if (artifact.source.kind.empty())
        {
            artifact.source.kind = "upload";
        }

        artifact.source.displayName = extractJsonString(*sourceRefObject, "displayName");
        const std::string sourcePath = extractJsonString(*sourceRefObject, "path");
        artifact.name = artifact.source.displayName.empty() ? "legacy.log" : artifact.source.displayName;

        if (!sourcePath.empty())
        {
            const foundation::Path absoluteSourcePath = foundation::Path(sourcePath);
            const foundation::Path artifactDirectory =
                foundation::Path(investigationDir.string() + "/artifacts/" + artifact.id);
            const foundation::Path dataPath = foundation::Path(artifactDirectory.string() + "/data");

            std::error_code errorCode;
            std::filesystem::create_directories(artifactDirectory.string(), errorCode);

            if (!errorCode)
            {
                std::filesystem::copy_file(absoluteSourcePath.string(), dataPath.string(),
                                         std::filesystem::copy_options::overwrite_existing, errorCode);
            }

            if (!errorCode)
            {
                artifact.relativePath = "artifacts/" + artifact.id + "/data";
            }
        }
    }

    if (artifact.name.empty())
    {
        artifact.name = "legacy.log";
    }

    if (artifact.relativePath.empty())
    {
        artifact.relativePath = "artifacts/" + artifact.id + "/data";
    }

    manifest.artifacts.push_back(std::move(artifact));
    manifest.primaryArtifactId = manifest.artifacts.front().id;

    return foundation::Result<InvestigationManifest>(std::move(manifest));
}

} // namespace scope::workspace
