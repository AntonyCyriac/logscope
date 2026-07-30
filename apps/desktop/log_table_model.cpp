/**
 * @file log_table_model.cpp
 */

#include "log_table_model.hpp"

#include <QString>

#include "log_line_classifier.hpp"

namespace scope::desktop
{

namespace
{

QString levelLabel(const scope::analysis::DetectedLogLevel level)
{
    switch (level)
    {
    case scope::analysis::DetectedLogLevel::Error:
        return QStringLiteral("ERROR");
    case scope::analysis::DetectedLogLevel::Warn:
        return QStringLiteral("WARN");
    case scope::analysis::DetectedLogLevel::Info:
        return QStringLiteral("INFO");
    case scope::analysis::DetectedLogLevel::Blank:
        return QStringLiteral("BLANK");
    case scope::analysis::DetectedLogLevel::Other:
    default:
        return QStringLiteral("OTHER");
    }
}

} // namespace

LogTableModel::LogTableModel(QObject* parent) : QAbstractTableModel(parent)
{
}

void LogTableModel::setLines(const std::vector<scope::analysis::IndexedLine>& lines)
{
    beginResetModel();
    m_lines = lines;
    m_rawTailLines.clear();
    m_tailStartLine = 0U;
    endResetModel();
}

void LogTableModel::appendRawLines(const std::vector<std::string>& rawLines, const std::uint64_t startLineNumber)
{
    if (rawLines.empty())
    {
        return;
    }

    const int first = static_cast<int>(m_lines.size() + m_rawTailLines.size());
    const int count = static_cast<int>(rawLines.size());

    beginInsertRows(QModelIndex(), first, first + count - 1);

    if (m_tailStartLine == 0U && !m_lines.empty())
    {
        m_tailStartLine = m_lines.back().lineNumber + 1U;
    }

    if (m_tailStartLine == 0U)
    {
        m_tailStartLine = startLineNumber;
    }

    for (const std::string& raw : rawLines)
    {
        m_rawTailLines.push_back(raw);
    }

    endInsertRows();
}

int LogTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return static_cast<int>(m_lines.size() + m_rawTailLines.size());
}

int LogTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return 4;
}

QVariant LogTableModel::data(const QModelIndex& index, const int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
    {
        return {};
    }

    const std::size_t row = static_cast<std::size_t>(index.row());

    if (row < m_lines.size())
    {
        const scope::analysis::IndexedLine& line = m_lines[row];

        switch (index.column())
        {
        case 0:
            return QVariant::fromValue(static_cast<qulonglong>(line.lineNumber));
        case 1:
            return line.timestamp.has_value() ? QString::fromStdString(line.timestamp->toString())
                                              : QString{};
        case 2:
            return levelLabel(line.level);
        case 3:
            return QString::fromStdString(line.messageExcerpt.empty() ? line.contentExcerpt : line.messageExcerpt);
        default:
            break;
        }
    }
    else
    {
        const std::size_t tailIndex = row - m_lines.size();
        const std::uint64_t lineNumber = m_tailStartLine + static_cast<std::uint64_t>(tailIndex);

        switch (index.column())
        {
        case 0:
            return QVariant::fromValue(static_cast<qulonglong>(lineNumber));
        case 1:
            return QString{};
        case 2:
            return QStringLiteral("TAIL");
        case 3:
            return QString::fromStdString(m_rawTailLines[tailIndex]);
        default:
            break;
        }
    }

    return {};
}

QVariant LogTableModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return {};
    }

    switch (section)
    {
    case 0:
        return QStringLiteral("Line");
    case 1:
        return QStringLiteral("Time");
    case 2:
        return QStringLiteral("Level");
    case 3:
        return QStringLiteral("Message");
    default:
        return {};
    }
}

} // namespace scope::desktop
