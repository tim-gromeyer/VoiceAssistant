#include <QString>

namespace strings {
inline float calculateSimilarity(const QString &str1, const QString &str2)
{
    using size = QString::size_type;

    const size len0 = str1.length() + 1;
    const size len1 = str2.length() + 1;

    if (str1.isEmpty() || str2.isEmpty()) {
        return 0.0;
    } else if (str1 == str2) {
        return 1.0;
    }

    std::vector<size> col(len1, 0);
    std::vector<size> prevCol(len1, 0);
    prevCol.reserve(len1);

    for (size i = 0; i < len1; i++) {
        prevCol[i] = i;
    }

    for (size i = 0; i < len0; i++) {
        col[0] = i;
        for (size j = 1; j < len1; j++) {
            col[j] = std::min(std::min(1 + col[j - 1], 1 + prevCol[j]),
                              prevCol[j - 1] + (i > 0 && str1[i - 1] == str2[j - 1] ? 0 : 1));
        }
        col.swap(prevCol);
    }

    const size dist = prevCol[len1 - 1];

    return 1.0F - float(dist) / (float) std::max(str1.length(), str2.length());
}
} // namespace strings
