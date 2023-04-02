#pragma once

#include <QString>
#include <QStringList>

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

namespace file {
QString makeSizeRedalbe(qint64);
}

namespace download {
QString makeSecoundsReadable(qint64);
QString makeDownloadSpeedReadable(qint64);
} // namespace download

namespace actions {
struct Action
{
    QString funcName;
    QStringList responses;

    // Execute program
    QString program;
    QStringList args;

    // Commands it reacts to
    QStringList commands;

    // A sound to play
    QString sound;

    void run(const QString &) const;
};
} // namespace actions
