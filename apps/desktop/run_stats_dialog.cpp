/**
 * @file run_stats_dialog.cpp
 */

#include "run_stats_dialog.hpp"

#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <sstream>

#include "stats_output.hpp"

namespace scope::desktop
{

RunStatsDialog::RunStatsDialog(const scope::analysis::AnalysisStats& analysisStats,
                               const scope::plugin::PluginLoadStats& pluginStats,
                               QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Run Statistics"));
    resize(480, 320);

    auto* layout = new QVBoxLayout(this);
    auto* text = new QTextEdit(this);
    text->setReadOnly(true);

    std::ostringstream stream;
    scope::cli::printRunStats(analysisStats, pluginStats, stream);
    text->setPlainText(QString::fromStdString(stream.str()));

    layout->addWidget(text);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

} // namespace scope::desktop
