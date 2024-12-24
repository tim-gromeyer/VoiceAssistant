#pragma once

#include "../base.h"

#include <QMainWindow>

class MediaControl : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_iid FILE "metadata.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit MediaControl(QObject *parent = nullptr)
        : QObject(parent){};

    bool isValid(const QString &) override;
    void run(const QString &) override;

private:
    using PluginInterface::bridge;

    static void nextTrack();
    static void previousTrack();
    static void play();
    static void pause();
    static void adjustVolume(int step);
    static void setVolume(int value);
    static int extractVolumeValue(const QString &text);
    static void controlMediaPlayer(const QString &command);
};
