#pragma once

#include <QCoreApplication>
#include <QMutex>
#include <QObject>

#if QT_WIDGETS_LIB
#include <QWidget>
#else
class QWidget;
#endif

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

        Q_EMIT _ask(text, QPrivateSignal());

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

        Q_EMIT _sayAndWait(text, QPrivateSignal());

        while (pause)
            QCoreApplication::processEvents();

        mutex.unlock();
    }

public Q_SLOTS:
    inline void say(const QString &text) { Q_EMIT _say(text, QPrivateSignal()); };

Q_SIGNALS:
    void _say(const QString &, QPrivateSignal);
    void _sayAndWait(const QString &, QPrivateSignal);
    void _ask(const QString &, QPrivateSignal);

    // void useWidget(QWidget *);

private:
    QString answer;
    bool pause = false;

    QMutex mutex;
};
