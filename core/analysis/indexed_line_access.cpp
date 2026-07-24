/**
 * @file indexed_line_access.cpp
 */

#include "indexed_line_access.hpp"

#include "index_reader.hpp"

namespace scope::analysis
{

bool hasQueryableIndex(const AnalysisModel& model) noexcept
{
    return model.lineIndex().has_value() || model.indexStore() != nullptr;
}

std::vector<IndexedLine> fetchIndexedLines(const AnalysisModel& model)
{
    const LineIndex* memoryIndex = model.lineIndex().has_value() ? &*model.lineIndex() : nullptr;
    const storage::IndexReader reader(memoryIndex, model.indexStore());

    return reader.lines();
}

std::uint64_t indexedLineCountForModel(const AnalysisModel& model) noexcept
{
    const LineIndex* memoryIndex = model.lineIndex().has_value() ? &*model.lineIndex() : nullptr;
    const storage::IndexReader reader(memoryIndex, model.indexStore());

    return reader.indexedLineCount();
}

std::uint64_t truncatedLineCountForModel(const AnalysisModel& model) noexcept
{
    const LineIndex* memoryIndex = model.lineIndex().has_value() ? &*model.lineIndex() : nullptr;
    const storage::IndexReader reader(memoryIndex, model.indexStore());

    return reader.truncatedLineCount();
}

} // namespace scope::analysis
