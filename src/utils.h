#pragma once

#include <QString>

#include <functional>

class QThread;

namespace utils {
inline namespace numbers {
int wordToNumber(const QString &);
}
namespace literals {
constexpr QLatin1String L1(const char *str)
{
    return QLatin1String{str, static_cast<int>(std::char_traits<char>::length(str))};
}
#define STR(str) QStringLiteral(str)
} // namespace literals
} // namespace utils

namespace threading {
void runFunctionInThreadPool(std::function<void()>);
QThread *runFunction(std::function<void()>);
}

namespace file {
QString makeSizeRedalbe(qint64);
}

namespace download {
QString makeSecoundsReadable(qint64);
QString makeDownloadSpeedReadable(qint64);
} // namespace download
