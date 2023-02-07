#include "global.h"

#include <QRunnable>
#include <QThreadPool>

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
    QThreadPool::globalInstance()->start(new FunctionRunnable(std::move(f)));
#else
    QThreadPool::globalInstance()->start(std::move(f));
#endif
}
} // namespace threading
