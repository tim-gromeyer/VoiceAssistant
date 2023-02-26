#pragma once

#include <QCoreApplication>
#include <QMutex>
#include <QObject>

class PluginBridge : public QObject
{
    Q_OBJECT

    friend class MainWindow;

public:
    explicit PluginBridge(QObject *parent = nullptr)
        : QObject(parent){};
    ~PluginBridge() { mutex.unlock(); }

    inline QString ask(const QString &text)
    {
        if (text.isEmpty())
            return {tr("Did you try to spy on the user?")};

        mutex.lock();
        pause = true;

        Q_EMIT _ask(text);

        while (pause)
            QCoreApplication::processEvents();

        mutex.unlock();
        return answer;
    };

    inline void sayAndWait(const QString &text)
    {
        if (text.isEmpty())
            return;

        mutex.lock();
        pause = true;

        Q_EMIT _sayAndWait(text);

        while (pause)
            QCoreApplication::processEvents();

        mutex.unlock();
    }

public Q_SLOTS:
    inline void say(const QString &text) { Q_EMIT _say(text); };

Q_SIGNALS:
    void _say(const QString &);
    void _sayAndWait(const QString &);
    void _ask(const QString &);

private:
    QString answer;
    bool pause = false;

    QMutex mutex;
};
