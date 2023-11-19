#pragma once

#include <QDir>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QString>

#if !QT_CONFIG(thread)
#error Threading required!
#endif

#ifdef QT_NO_DEBUG
#include <QCoreApplication>
#endif

#define NEED_MICROPHONE_PERMISSION (QT_FEATURE_permissions == 1)

namespace dir {
static const QString &baseDir()
{
#ifdef QT_DEBUG
    static const QString dir = QStringLiteral(APP_DIR);
#else
    static QString dir;

    if (QCoreApplication::instance() && dir.isEmpty()) {
#if defined(Q_OS_LINUX) || defined(Q_OS_WIN)
        if (QCoreApplication::applicationDirPath().endsWith(QLatin1String("bin")))
#if QT5
            dir = QLibraryInfo::location(QLibraryInfo::PrefixPath);
#else
            dir = QLibraryInfo::path(QLibraryInfo::PrefixPath);
#endif
        else
            dir = QCoreApplication::applicationDirPath();
#endif
    }

#endif

    return dir;
}
static const QString &dataDir()
{
    static const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir;
}

static const QString &pluginDir()
{
    static const QString dir = baseDir() + QStringLiteral("/plugins/");
    return dir;
}
static const QString &modelDir()
{
    static const QString dir = dataDir() + QStringLiteral("/models/");
    return dir;
}
static const QString &commandsBaseDir()
{
    static const QString dir = dataDir() + QStringLiteral("/commands/");
    return dir;
}
static const QString &speechToTextPluginDir()
{
    static const QString dir = baseDir() + QStringLiteral("/speechtotext/");
    return dir;
}
static const QString &commandsInstallBaseDir()
{
    static const QString dir = baseDir() + QStringLiteral("/commands/");
    return dir;
}
} // namespace dir
