#pragma once

#include <QString>
#include <QStringList>

#include <algorithm>
#include <string>
#include <vector>

namespace utils {
inline namespace numbers {
int wordToNumber(const QString &);
}
namespace strings {
QString normalizeText(const QString &);

namespace literals {
constexpr QLatin1String L1(const char *str)
{
    return QLatin1String{str, static_cast<int>(std::char_traits<char>::length(str))};
}
#define STR(str) QStringLiteral(str)
} // namespace literals
} // namespace strings
} // namespace utils

namespace directory {
bool copyRecursively(const QString &fromDir, const QString &toDir, bool coverFileIfExist = false);
} // namespace directory

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
    // Name of the action
    QString name;

    // Is this a user action or a default action
    bool isUserAction = true;

    // Name of the function to execute (optional)
    QString funcName;
    // A list of possible responses. A random response is chosen
    QStringList responses;

    // Path/Name od the executable to run
    QString program;
    // Arguments passed to the executable
    QStringList args;

    // Commands it reacts to
    QStringList commands;

    // A sound to play
    QString sound;

    void run(QStringView) const;
};
} // namespace actions
