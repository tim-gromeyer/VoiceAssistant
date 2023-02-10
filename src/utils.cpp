#include "utils.h"

#include <QCoreApplication>
#include <QHash>
#include <QRunnable>
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

namespace threading {
void runFunction(std::function<void()> f)
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
} // namespace threading
