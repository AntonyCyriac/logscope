/**
 * @file timeline_load_worker.hpp
 */

#pragma once

#include <QObject>

#include <QtGlobal>

#include "foundation/path.hpp"
#include "timeline_event.hpp"
#include "workspace.hpp"

namespace scope::desktop
{

class TimelineLoadWorker : public QObject
{
    Q_OBJECT

  public:
    explicit TimelineLoadWorker(QObject* parent = nullptr);

  public slots:
    void load(const QString& investigationDirectoryPath, quint64 limit, quint64 offset);

  signals:
    void loadFinished(scope::workspace::TimelineProjectionResult result);
    void loadFailed(const QString& message);
};

} // namespace scope::desktop
