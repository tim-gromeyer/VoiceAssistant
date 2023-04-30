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
// Implementation by ChatGPT
inline double calculateSimilarity(const std::string &str1, const std::string &str2)
{
    int n = (int) str1.length();
    int m = (int) str2.length();

    if (n == 0 || m == 0)
        return 0.0;
    else if (str1 == str2)
        return 1.0;

    std::vector<std::vector<int>> dist(n + 1, std::vector<int>(m + 1));

    for (int i = 0; i <= n; ++i) {
        dist[i][0] = i;
    }
    for (int j = 0; j <= m; ++j) {
        dist[0][j] = j;
    }

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
            dist[i][j] = std::min(dist[i - 1][j] + 1,
                                  std::min(dist[i][j - 1] + 1, dist[i - 1][j - 1] + cost));
        }
    }

    double ratio = 1.0 - (double) dist[n][m] / std::max(n, m);
    return std::max(ratio, 0.0);
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
