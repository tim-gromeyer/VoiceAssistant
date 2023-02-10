#pragma once

#include <QDir>
#include <QString>

#if !QT_CONFIG(thread)
#error Threading required!
#endif

#define NEED_MICROPHONE_PERMISSION (QT_FEATURE_permissions == 1)
