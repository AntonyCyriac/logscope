/**
 * @file crash_load_worker.hpp
 */

#pragma once

#include <QObject>

#include "crash_report.hpp"
#include "foundation/path.hpp"

namespace scope::desktop
{

class CrashLoadWorker : public QObject
{
    Q_OBJECT

  public:
    explicit CrashLoadWorker(QObject* parent = nullptr);

  public slots:
    void load(const QString& investigationDirectoryPath, const QString& artifactId);

  signals:
    void loadFinished(scope::workspace::CrashReport report);
    void loadFailed(const QString& message);
};

} // namespace scope::desktop
