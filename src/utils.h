#pragma once

#include <QString>
#include <QStringList>

namespace utils {
inline namespace numbers {
int wordToNumber(const QString &);
}
namespace strings {
// Implementation by ChatGPT
inline double calculateSimilarity(const QString &str1, const QString &str2)
{
    int n = str1.length();
    int m = str2.length();

    if (n == 0 || m == 0)
        return 0.0;
    else if (str1 == str2)
        return 1.0;

    QVector<QVector<int>> dist(n + 1, QVector<int>(m + 1));

    for (int i = 0; i <= n; ++i) {
        dist[i][0] = i;
    }
    for (int j = 0; j <= m; ++j) {
        dist[0][j] = j;
    }

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
            dist[i][j] = qMin(dist[i - 1][j] + 1,
                              qMin(dist[i][j - 1] + 1, dist[i - 1][j - 1] + cost));
        }
    }

    double ratio = 1.0 - (double) dist[n][m] / qMax(n, m);
    return qMax(ratio, 0.0);
}

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
