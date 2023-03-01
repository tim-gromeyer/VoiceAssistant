#include "utils.h"

#include <QCoreApplication>
#include <QHash>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>

namespace utils {
inline namespace numbers {
int wordToNumber(const QString &word)
{
    // Maybe reading from a JSON file is a better approach
    static QHash<QString, int> numbers;
    if (numbers.isEmpty()) {
        numbers[QCoreApplication::translate("number", "zero")] = 0;
        numbers[QCoreApplication::translate("number", "one")] = 1;
        numbers[QCoreApplication::translate("number", "two")] = 2;
        numbers[QCoreApplication::translate("number", "three")] = 3;
        numbers[QCoreApplication::translate("number", "four")] = 4;
        numbers[QCoreApplication::translate("number", "five")] = 5;
        numbers[QCoreApplication::translate("number", "six")] = 6;
        numbers[QCoreApplication::translate("number", "seven")] = 7;
        numbers[QCoreApplication::translate("number", "eight")] = 8;
        numbers[QCoreApplication::translate("number", "nine")] = 9;
        numbers[QCoreApplication::translate("number", "ten")] = 10;
    }

    return numbers.value(word, 10);
}
} // namespace numbers
} // namespace utils

class FunctionRunnable : public QRunnable
{
    std::function<void()> m_functionToRun;

public:
    explicit FunctionRunnable(std::function<void()> functionToRun)
        : m_functionToRun(std::move(functionToRun))
    {}
    void run() override { m_functionToRun(); }
};
class FunctionThread : public QThread
{
    std::function<void()> m_functionToRun;

public:
    explicit FunctionThread(std::function<void()> functionToRun)
        : m_functionToRun(std::move(functionToRun))
    {}
    void run() override { m_functionToRun(); }
};
namespace threading {
void runFunctionInThreadPool(std::function<void()> f)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    if (!f)
        return;

    QRunnable *runnable = new FunctionRunnable(std::move(f));
    QThreadPool::globalInstance()->start(runnable);
#else
    QThreadPool::globalInstance()->start(std::move(f));
#endif
}
QThread *runFunction(std::function<void()> f)
{
    auto *thread = new FunctionThread(std::move(f));
    return thread;
}
} // namespace threading

namespace file {
QString makeSizeRedalbe(qint64 size)
{
    QString sizeString;
    if (size < 1024) {
        sizeString = QString::number(size) + " B";
    } else if (size < 1048576) {
        sizeString = QString::number(size / 1024.0, 'f', 2) + " KB";
    } else {
        sizeString = QString::number(size / 1048576.0, 'f', 2) + " MB";
    }

    return sizeString;
}
} // namespace file

namespace download {
QString makeSecoundsReadable(qint64 secondsRemaining)
{
    QString timeString;

    // Convert seconds remaining to a more readable format
    if (secondsRemaining < 60) {
        timeString = QCoreApplication::translate("time", "%1 seconds")
                         .arg(QString::number(secondsRemaining));
    } else if (secondsRemaining < 3600) {
        qint64 minutes = secondsRemaining / 60;
        qint64 seconds = secondsRemaining % 60;
        timeString = QCoreApplication::translate("time", "%1 minute(s), %2 secounds")
                         .arg(QString::number(minutes), QString::number(seconds));
    } else if (secondsRemaining < 86400) {
        qint64 hours = secondsRemaining / 3600;
        qint64 minutes = (secondsRemaining % 3600) / 60;
        timeString = QCoreApplication::translate("time", "%1 hour(s), %2 minute(s)")
                         .arg(QString::number(hours), QString::number(minutes));
    } else if (secondsRemaining < 31536000) {
        qint64 days = secondsRemaining / 86400;
        qint64 hours = (secondsRemaining % 86400) / 3600;
        timeString = QCoreApplication::translate("time", "%1 day(s), %2 hour(s)")
                         .arg(QString::number(days), QString::number(hours));
    } else {
        qint64 years = secondsRemaining / 31536000;
        qint64 days = (secondsRemaining % 31536000) / 86400;
        timeString = QCoreApplication::translate("time", "%1 year(s), %2 day(s)")
                         .arg(QString::number(years), QString::number(days));
    }

    return timeString;
}
QString makeDownloadSpeedReadable(qint64 downloadSpeed)
{
    return file::makeSizeRedalbe(downloadSpeed) + STR("/s");
}
} // namespace download
