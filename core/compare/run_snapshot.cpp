/**
 * @file run_snapshot.cpp
 * @brief Run snapshot implementation.
 */

#include "run_snapshot.hpp"

#include "indexed_line_access.hpp"
#include "instance_grouper.hpp"
#include "message_signature.hpp"

#include "foundation/filesystem.hpp"

namespace scope::compare
{

RunSnapshot buildRunSnapshot(const analysis::AnalysisModel& model)
{
    RunSnapshot snapshot;

    if (model.discoveryCensus().has_value())
    {
        snapshot.discovery = *model.discoveryCensus();
    }

    if (model.analysisAccounting().has_value())
    {
        snapshot.analysis = *model.analysisAccounting();
    }

    const auto isDirectory = foundation::FileSystem::isDirectory(snapshot.discovery.root);

    snapshot.rootIsFile = isDirectory.hasValue() && !*isDirectory;

    if (!analysis::hasQueryableIndex(model))
    {
        return snapshot;
    }

    for (const analysis::IndexedLine& line : analysis::fetchIndexedLines(model))
    {
        if (line.level != analysis::DetectedLogLevel::Error || line.messageExcerpt.empty())
        {
            continue;
        }

        const std::string signature = analytics::normalizeClusterSignature(line.messageExcerpt);

        if (signature.empty())
        {
            continue;
        }

        SignatureEntry& entry = snapshot.signatures[signature];

        if (entry.signature.empty())
        {
            entry.signature = signature;
            entry.sampleMessage = line.messageExcerpt;
            entry.sampleLocation.sourceFileRelative = line.sourceFileRelative;
            entry.sampleLocation.fileLineNumber = line.fileLineNumber;
            entry.sampleLocation.instanceKey = source::deriveInstanceKey(line.sourceFileRelative);

            if (line.timestamp.has_value())
            {
                entry.firstSeen = line.timestamp->toString();
            }
        }

        ++entry.count;
    }

    return snapshot;
}

} // namespace scope::compare
