#pragma once

#include <QObject>

class Settings final : public QObject
{
    Q_OBJECT

    friend class MainWindow;

public:
    Settings *instance();

private:
    explicit Settings(QObject *parent = nullptr);
    ~Settings() final;
};
