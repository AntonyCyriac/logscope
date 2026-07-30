/**
 * @file tail_worker.hpp
 * @brief Background worker for live tail polling.
 */

#pragma once

#include <QObject>
#include <QTimer>

#include "application_service.hpp"

namespace scope::desktop
{

/**
 * @brief Polls ApplicationService for tail lines on a timer.
 */
class TailWorker : public QObject
{
    Q_OBJECT

  public:
    explicit TailWorker(scope::application::ApplicationService* service, QObject* parent = nullptr);

    void start();
    void stop();

  signals:
    void linesAppended(const QStringList& lines);
    void tailError(const QString& message);

  private:
    void poll();

    scope::application::ApplicationService* m_service{nullptr};
    QTimer m_timer;
};

} // namespace scope::desktop
