#pragma once

#include <QDir>
#include <QString>

#if !QT_CONFIG(thread)
#error Threading required!
#endif

#define NEED_MICROPHONE_PERMISSION (QT_FEATURE_permissions == 1)

namespace dir {
static const QString &baseDir()
{
#ifdef QT_DEBUG
    static const QString dir = QStringLiteral(APP_DIR);
#else
    static const QString dir = QCoreApplication::applicationDirPath();
#endif

    return dir;
}
static const QString &dataDir()
{
    static const QString dir = baseDir() + QStringLiteral("/data/");
    return dir;
}

static const QString &pluginDir()
{
    static const QString dir = baseDir() + QStringLiteral("/plugins/");
    return dir;
}
static const QString &modelDir()
{
    static const QString dir = baseDir() + QStringLiteral("/models/");
    return dir;
}
static const QString &speechToTextPluginDir()
{
    static const QString dir = baseDir() + QStringLiteral("/speechtotext/");
    return dir;
}
} // namespace dir
