/**
 * @file timeline_load_worker.cpp
 */

#include "timeline_load_worker.hpp"

#include "investigation_container.hpp"

namespace scope::desktop
{

TimelineLoadWorker::TimelineLoadWorker(QObject* parent) : QObject(parent) {}

void TimelineLoadWorker::load(const QString& investigationDirectoryPath,
                              scope::workspace::TimelineProjectionOptions options)
{
    const auto investigationResult =
        scope::workspace::Investigation::open(scope::foundation::Path(investigationDirectoryPath.toStdString()));

    if (!investigationResult)
    {
        emit loadFailed(QString::fromStdString(investigationResult.error().message()));

        return;
    }

    const auto timelineResult = investigationResult->projectTimeline(options);

    if (!timelineResult)
    {
        emit loadFailed(QString::fromStdString(timelineResult.error().message()));

        return;
    }

    emit loadFinished(*timelineResult);
}

} // namespace scope::desktop
