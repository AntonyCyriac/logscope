/**
 * @file timeline_load_worker.cpp
 */

#include "timeline_load_worker.hpp"

#include <QMetaType>

#include "investigation_container.hpp"

namespace scope::desktop
{

namespace
{

const int kTimelineResultMetaType = []() {
    qRegisterMetaType<scope::workspace::TimelineProjectionResult>("scope::workspace::TimelineProjectionResult");

    return 0;
}();

} // namespace

TimelineLoadWorker::TimelineLoadWorker(QObject* parent) : QObject(parent)
{
    Q_UNUSED(kTimelineResultMetaType);
}

void TimelineLoadWorker::load(const QString& investigationDirectoryPath, const quint64 limit,
                              const quint64 offset)
{
    const auto investigationResult =
        scope::workspace::Investigation::open(scope::foundation::Path(investigationDirectoryPath.toStdString()));

    if (!investigationResult)
    {
        emit loadFailed(QString::fromStdString(investigationResult.error().message()));

        return;
    }

    scope::workspace::TimelineProjectionOptions options;
    options.limit = static_cast<std::size_t>(limit);
    options.offset = static_cast<std::size_t>(offset);

    const auto timelineResult = investigationResult->projectTimeline(options);

    if (!timelineResult)
    {
        emit loadFailed(QString::fromStdString(timelineResult.error().message()));

        return;
    }

    emit loadFinished(*timelineResult);
}

} // namespace scope::desktop
