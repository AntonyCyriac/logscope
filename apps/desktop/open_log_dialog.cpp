/**
 * @file open_log_dialog.cpp
 */

#include "open_log_dialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace scope::desktop
{

OpenLogDialog::OpenLogDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Open Log"));
    buildUi();
}

void OpenLogDialog::buildUi()
{
    auto* layout = new QVBoxLayout(this);

    auto* pathRow = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    auto* browseButton = new QPushButton(QStringLiteral("Browse…"), this);
    pathRow->addWidget(m_pathEdit);
    pathRow->addWidget(browseButton);
    layout->addLayout(pathRow);

    auto* form = new QFormLayout();
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItems({QStringLiteral("auto"), QStringLiteral("plain"), QStringLiteral("jsonl")});
    form->addRow(QStringLiteral("Log format"), m_formatCombo);

    m_profileCombo = new QComboBox(this);
    m_profileCombo->addItem(QStringLiteral("(none)"), QString{});
    m_profileCombo->addItem(QStringLiteral("generic-plain"), QStringLiteral("generic-plain"));
    m_profileCombo->addItem(QStringLiteral("generic-json"), QStringLiteral("generic-json"));
    form->addRow(QStringLiteral("Profile"), m_profileCombo);

    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(browseButton, &QPushButton::clicked, this, [this]() {
        const QString path =
            QFileDialog::getOpenFileName(this, QStringLiteral("Open log file"), m_pathEdit->text(),
                                         QStringLiteral("Logs (*.log *.jsonl *.*)"));

        if (!path.isEmpty())
        {
            m_pathEdit->setText(path);
        }
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString OpenLogDialog::logPath() const
{
    return m_pathEdit->text().trimmed();
}

scope::analysis::LogFormat OpenLogDialog::logFormat() const
{
    const QString format = m_formatCombo->currentText();

    if (format == QStringLiteral("plain"))
    {
        return scope::analysis::LogFormat::PlainText;
    }

    if (format == QStringLiteral("jsonl"))
    {
        return scope::analysis::LogFormat::JsonLines;
    }

    return scope::analysis::LogFormat::Auto;
}

std::string OpenLogDialog::profile() const
{
    return m_profileCombo->currentData().toString().toStdString();
}

} // namespace scope::desktop
