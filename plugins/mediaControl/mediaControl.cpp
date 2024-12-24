#include "mediaControl.h"

#include <QLabel>
#include <QProcess>
#include <QVBoxLayout>
#ifdef HAS_DBUS
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#endif

// Define a configurable step for volume adjustment
static const int VOLUME_STEP = 5; // Change volume in 5% steps

bool MediaControl::isValid(const QString &text)
{
    return text.contains("lied") || text.contains("pause") || text.contains("weiter")
           || text.contains("lautstärke") || text.contains("lauter") || text.contains("leiser");
}

void MediaControl::run(const QString &text)
{
    if (text.contains("nächstes lied")) {
        nextTrack();
    } else if (text.contains("vorheriges lied")) {
        previousTrack();
    } else if (text.contains("play") || text.contains("wiedergabe") || text.contains("weiter")) {
        play();
    } else if (text.contains("pause") || text.contains("pausieren")) {
        pause();
    } else if (text.contains("lauter") || text.contains("lautstärke hoch")) {
        adjustVolume(VOLUME_STEP);
    } else if (text.contains("leiser") || text.contains("lautstärke runter")) {
        adjustVolume(-VOLUME_STEP);
    } else if (text.contains("lautstärke auf")) {
        int value = extractVolumeValue(text);
        if (value >= 0 && value <= 100) {
            setVolume(value);
        }
    }
}

void MediaControl::nextTrack()
{
    controlMediaPlayer("Next");
}

void MediaControl::previousTrack()
{
    controlMediaPlayer("Previous");
}

void MediaControl::play()
{
    controlMediaPlayer("Play");
}

void MediaControl::pause()
{
    controlMediaPlayer("Pause");
}

void MediaControl::controlMediaPlayer(const QString &command)
{
#if defined(Q_OS_LINUX) && HAS_DBUS
    // Try MPRIS D-Bus interface for media players
    QDBusConnection bus = QDBusConnection::sessionBus();

    // Get list of available MPRIS players
    QDBusInterface iface("org.freedesktop.DBus",
                         "/org/freedesktop/DBus",
                         "org.freedesktop.DBus",
                         bus);

    QDBusReply<QStringList> reply = iface.call("ListNames");
    if (reply.isValid()) {
        QStringList services = reply.value();

        // Find first available media player
        for (const QString &service : services) {
            if (service.startsWith("org.mpris.MediaPlayer2.")) {
                QDBusInterface player(service,
                                      "/org/mpris/MediaPlayer2",
                                      "org.mpris.MediaPlayer2.Player",
                                      bus);
                player.call(command);
                break;
            }
        }
    }
#elif defined(Q_OS_MACOS)
    QString script;
    if (command == "Next") {
        script = "tell application \"System Events\" to key code 124 using {command down}";
    } else if (command == "Previous") {
        script = "tell application \"System Events\" to key code 123 using {command down}";
    } else if (command == "Play") {
        script = "tell application \"System Events\" to key code 49 using {command down}";
    } else if (command == "Pause") {
        script = "tell application \"System Events\" to key code 49 using {command down}";
    }
    QProcess::execute("osascript", QStringList() << "-e" << script);

#elif defined(Q_OS_WINDOWS)
    QChar mediaKey;
    if (command == "Next") {
        mediaKey = QChar(0xB0); // Next track
    } else if (command == "Previous") {
        mediaKey = QChar(0xB1); // Previous track
    } else if (command == "Play") {
        mediaKey = QChar(0xB3); // Play/Pause
    } else if (command == "Pause") {
        mediaKey = QChar(0xB3); // Play/Pause
    }
    QProcess::execute("powershell",
                      QStringList()
                          << "-command"
                          << QString("(New-Object -ComObject WScript.Shell).SendKeys([char]0x%1)")
                                 .arg(mediaKey.unicode(), 0, 16));

#elif defined(Q_OS_ANDROID)
    QString keyEvent;
    if (command == "Next") {
        keyEvent = "87"; // KEYCODE_MEDIA_NEXT
    } else if (command == "Previous") {
        keyEvent = "88"; // KEYCODE_MEDIA_PREVIOUS
    } else if (command == "Play" || command == "Pause") {
        keyEvent = "85"; // KEYCODE_MEDIA_PLAY_PAUSE
    }
    QProcess::execute("am",
                      QStringList() << "broadcast"
                                    << "-a"
                                    << "android.intent.action.MEDIA_BUTTON"
                                    << "-e"
                                    << "android.intent.extra.KEY_EVENT" << keyEvent);
#endif
}

void MediaControl::adjustVolume(int step)
{
#if defined(Q_OS_LINUX) && HAS_DBUS
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface iface("org.freedesktop.DBus",
                         "/org/freedesktop/DBus",
                         "org.freedesktop.DBus",
                         bus);

    QDBusReply<QStringList> reply = iface.call("ListNames");
    if (reply.isValid()) {
        QStringList services = reply.value();
        for (const QString &service : services) {
            if (service.startsWith("org.mpris.MediaPlayer2.")) {
                QDBusInterface player(service,
                                      "/org/mpris/MediaPlayer2",
                                      "org.mpris.MediaPlayer2.Player",
                                      bus);
                player.call("Volume", step > 0 ? "Raise" : "Lower", qAbs(step));
                break;
            }
        }
    }
#elif defined(Q_OS_MACOS)
    QString script = QString(
                         "set volume output volume (output volume of (get volume settings) + %1)")
                         .arg(step);
    QProcess::execute("osascript", QStringList() << "-e" << script);

#elif defined(Q_OS_WINDOWS)
    QProcess::execute("nircmdc.exe",
                      QStringList() << (step > 0 ? "changesysvolume" : "changesysvolume")
                                    << QString::number(step * 655.35));

#elif defined(Q_OS_ANDROID)
    QString keyEvent = (step > 0) ? "24" : "25"; // KEYCODE_VOLUME_UP / KEYCODE_VOLUME_DOWN
    for (int i = 0; i < qAbs(step); ++i) {
        QProcess::execute("input", QStringList() << "keyevent" << keyEvent);
    }
#endif
}

void MediaControl::setVolume(int value)
{
#if defined(Q_OS_LINUX) && HAS_DBUS
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface iface("org.freedesktop.DBus",
                         "/org/freedesktop/DBus",
                         "org.freedesktop.DBus",
                         bus);

    QDBusReply<QStringList> reply = iface.call("ListNames");
    if (reply.isValid()) {
        QStringList services = reply.value();
        for (const QString &service : services) {
            if (service.startsWith("org.mpris.MediaPlayer2.")) {
                QDBusInterface player(service,
                                      "/org/mpris/MediaPlayer2",
                                      "org.mpris.MediaPlayer2.Player",
                                      bus);
                player.call("SetVolume", value / 100.0);
                break;
            }
        }
    }
#elif defined(Q_OS_MACOS)
    QString script = QString("set volume output volume %1").arg(value);
    QProcess::execute("osascript", QStringList() << "-e" << script);

#elif defined(Q_OS_WINDOWS)
    QProcess::execute("nircmdc.exe",
                      QStringList() << "setsysvolume" << QString::number(value * 655.35));

#elif defined(Q_OS_ANDROID)
    // Android does not natively support setting a specific volume through shell
    // commands. Requires direct integration with Android APIs or rooted device.
    // Placeholder for now.
    qDebug() << "Setting specific volume is not supported on Android through shell commands.";
#endif
}
