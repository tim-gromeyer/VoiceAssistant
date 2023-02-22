#pragma once

#include <QDebug>
#include <QObject>

class PluginBridge : public QObject
{
    Q_OBJECT
public:
    explicit PluginBridge(QObject *parent = nullptr)
        : QObject(parent){};

    inline QString ask(const QString &text)
    {
        Q_EMIT _ask(text);

        return answer;
    };

    inline void sayAndWait(const QString &text) { Q_EMIT _sayAndWait(text); }

public Q_SLOTS:
    inline void say(const QString &text) { Q_EMIT _say(text); };
    inline void _setAnswer(const QString &newAnswer) { answer = newAnswer; }

Q_SIGNALS:
    void _say(const QString &);
    void _sayAndWait(const QString &);
    void _ask(const QString &);

private:
    QString answer;
};
