/**
 * @file crash_load_worker.cpp
 */

#include "crash_load_worker.hpp"

#include "investigation_container.hpp"

namespace scope::desktop
{

CrashLoadWorker::CrashLoadWorker(QObject* parent) : QObject(parent) {}

void CrashLoadWorker::load(const QString& investigationDirectoryPath, const QString& artifactId)
{
    const auto investigationResult =
        scope::workspace::Investigation::open(scope::foundation::Path(investigationDirectoryPath.toStdString()));

    if (!investigationResult)
    {
        emit loadFailed(QString::fromStdString(investigationResult.error().message()));

        return;
    }

    const auto crashResult = investigationResult->analyzeCrash(artifactId.toStdString());

    if (!crashResult)
    {
        emit loadFailed(QString::fromStdString(crashResult.error().message()));

        return;
    }

    emit loadFinished(*crashResult);
}

} // namespace scope::desktop
