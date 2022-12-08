#pragma once

#include <QFileInfo>
#include <QString>

namespace literals {
constexpr std::size_t length(const char* str)
{
    return std::char_traits<char>::length(str);
}

constexpr QLatin1String make_latin1(const char* str)
{
    return QLatin1String{str, static_cast<int>(length(str))};
}
} // namespace literals

// QLatin1String literal
# define L1(str) literals::make_latin1(str)
# define STR(str) QStringLiteral(str)

namespace dir {
inline bool exists(const QString &dir) {
    return QFileInfo(dir).isDir();
}
} // namespace dir
