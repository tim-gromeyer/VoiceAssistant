#pragma once

#include <QDir>
#include <QString>

namespace literals {
constexpr QLatin1String L1(const char *str)
{
    return QLatin1String{str, static_cast<int>(std::char_traits<char>::length(str))};
}
#define STR(str) QStringLiteral(str)
} // namespace literals

namespace dir {
inline bool exists(const QString &dir)
{
    return QDir(dir).exists();
}
} // namespace dir
