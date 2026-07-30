/**
 * @file log_table_model.hpp
 * @brief Table model for virtualized log line display.
 */

#pragma once

#include <QAbstractTableModel>
#include <vector>

#include "line_index.hpp"

namespace scope::desktop
{

/**
 * @brief Displays indexed log lines in a table view.
 */
class LogTableModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    explicit LogTableModel(QObject* parent = nullptr);

    void setLines(const std::vector<scope::analysis::IndexedLine>& lines);

    void appendRawLines(const std::vector<std::string>& rawLines, std::uint64_t startLineNumber);

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  private:
    std::vector<scope::analysis::IndexedLine> m_lines;
    std::vector<std::string> m_rawTailLines;
    std::uint64_t m_tailStartLine{0U};
};

} // namespace scope::desktop
