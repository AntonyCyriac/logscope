/**
 * @file attributed_ingest_source.cpp
 * @brief AttributedIngestSource implementation.
 */

#include "attributed_ingest_source.hpp"

#include "file_log_source.hpp"
#include "foundation/error.hpp"
#include "log_macros.hpp"

namespace scope::source
{

AttributedIngestSource::AttributedIngestSource(foundation::Path displayPath, std::vector<IngestStream> streams)
    : m_displayPath(std::move(displayPath)), m_streams(std::move(streams))
{
}

foundation::Result<std::unique_ptr<LogSource>> AttributedIngestSource::create(foundation::Path displayPath,
                                                                              std::vector<IngestStream> streams)
{
    if (streams.empty())
    {
        return foundation::Result<std::unique_ptr<LogSource>>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No ingest streams to open."));
    }

    auto source = std::unique_ptr<LogSource>(new AttributedIngestSource(std::move(displayPath), std::move(streams)));
    auto* attributed = static_cast<AttributedIngestSource*>(source.get());
    const auto advanceResult = attributed->advanceFile();

    if (!advanceResult)
    {
        return foundation::Result<std::unique_ptr<LogSource>>(advanceResult.error());
    }

    return foundation::Result<std::unique_ptr<LogSource>>(std::move(source));
}

const foundation::Path& AttributedIngestSource::path() const noexcept
{
    return m_displayPath;
}

foundation::Result<bool> AttributedIngestSource::advanceFile()
{
    while (m_streamIndex < m_streams.size())
    {
        IngestStream& stream = m_streams[m_streamIndex];

        while (m_fileIndex < stream.orderedFiles.size())
        {
            const IngestFile& file = stream.orderedFiles[m_fileIndex];
            auto openResult = FileLogSource::open(file.absolutePath);

            if (!openResult)
            {
                return foundation::Result<bool>(openResult.error());
            }

            m_currentFile = std::move(*openResult);
            m_fileLineNumber = 0U;

            SCOPE_LOG_DEBUG("source.ingest", "Opened ingest file: " + file.relativePath);

            return foundation::Result<bool>(true);
        }

        ++m_streamIndex;
        m_fileIndex = 0U;
    }

    m_currentFile.reset();

    return foundation::Result<bool>(true);
}

foundation::Result<bool> AttributedIngestSource::readLine(std::string& line)
{
    while (m_currentFile != nullptr)
    {
        const auto readResult = m_currentFile->readLine(line);

        if (!readResult)
        {
            return readResult;
        }

        if (!*readResult)
        {
            ++m_fileIndex;
            const auto advanceResult = advanceFile();

            if (!advanceResult)
            {
                return advanceResult;
            }

            if (m_currentFile == nullptr)
            {
                break;
            }

            continue;
        }

        ++m_fileLineNumber;
        ++m_streamLineNumber;

        LineAttribution attribution;
        attribution.sourceFile = m_currentFile->path();
        attribution.sourceFileRelative = m_streams[m_streamIndex].orderedFiles[m_fileIndex].relativePath;
        attribution.fileLineNumber = m_fileLineNumber;
        attribution.streamLineNumber = m_streamLineNumber;
        m_lastAttribution = attribution;

        return foundation::Result<bool>(true);
    }

    m_lastAttribution.reset();

    return foundation::Result<bool>(false);
}

std::optional<LineAttribution> AttributedIngestSource::lastLineAttribution() const noexcept
{
    return m_lastAttribution;
}

bool isAttributedLogSource(const LogSource& source) noexcept
{
    return dynamic_cast<const AttributedIngestSource*>(&source) != nullptr;
}

const AttributedIngestSource* asAttributedLogSource(const LogSource& source) noexcept
{
    return dynamic_cast<const AttributedIngestSource*>(&source);
}

} // namespace scope::source
