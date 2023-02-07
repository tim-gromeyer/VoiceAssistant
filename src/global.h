#pragma once

#include <QDir>
#include <QString>

#include <functional>

namespace literals {
constexpr QLatin1String L1(const char *str)
{
    return QLatin1String{str, static_cast<int>(std::char_traits<char>::length(str))};
}
#define STR(str) QStringLiteral(str)
} // namespace literals

namespace threading {
void runFunction(std::function<void()>);
}

#define NEED_MICROPHONE_PERMISSION QT_FEATURE_permissions == 1
