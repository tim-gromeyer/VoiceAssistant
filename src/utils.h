#pragma once

#include <QString>
#include <QStringList>

#include <algorithm>
#include <string>
#include <vector>

namespace utils {
inline namespace numbers {
int wordToNumber(QString);
}
namespace strings {
inline double calculateSimilarity(const std::string &str1, const std::string &str2)
{
    const size_t len0 = str1.size() + 1;
    const size_t len1 = str2.size() + 1;

    if (str1.empty() || str2.empty()) {
        return 0;
    }

    std::vector<size_t> col(len1, 0);
    std::vector<size_t> prevCol(len1, 0);

    for (size_t i = 0; i < len1; i++) {
        prevCol[i] = i;
    }

    for (size_t i = 0; i < len0; i++) {
        col[0] = i;
        for (size_t j = 1; j < len1; j++) {
            col[j] = std::min(std::min(1 + col[j - 1], 1 + prevCol[j]),
                              prevCol[j - 1] + (i > 0 && str1[i - 1] == str2[j - 1] ? 0 : 1));
        }
        col.swap(prevCol);
    }

    const size_t dist = prevCol[len1 - 1];

    return 1.0F - float(dist / std::max(str1.size(), str2.size()));
}
inline double calculateSimilarity(const QString &str1, const QString &str2)
{
    return calculateSimilarity(str1.toStdString(), str2.toStdString());
}
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
