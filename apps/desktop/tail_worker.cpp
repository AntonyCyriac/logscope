/**
 * @file tail_worker.cpp
 */

#include "tail_worker.hpp"

#include <QStringList>

namespace scope::desktop
{

TailWorker::TailWorker(scope::application::ApplicationService* service, QObject* parent)
    : QObject(parent), m_service(service)
{
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, &TailWorker::poll);
}

void TailWorker::start()
{
    m_timer.start();
}

void TailWorker::stop()
{
    m_timer.stop();
}

void TailWorker::poll()
{
    if (m_service == nullptr)
    {
        return;
    }

    const auto linesResult = m_service->pollTailLines();

    if (!linesResult)
    {
        emit tailError(QString::fromStdString(linesResult.error().message()));

        return;
    }

    if (linesResult->empty())
    {
        return;
    }

    QStringList qLines;

    for (const std::string& line : *linesResult)
    {
        qLines.push_back(QString::fromStdString(line));
    }

    emit linesAppended(qLines);
}

} // namespace scope::desktop
