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

#define NEED_MICROPHONE_PERMISSION QT_FEATURE_permissions == 1
