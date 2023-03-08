#include "settings.h"

Settings *_settingsInstance = nullptr;

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    _settingsInstance = this;
}

Settings *Settings::instance()
{
    return _settingsInstance;
}

Settings::~Settings()
{
    _settingsInstance->deleteLater();
    delete _settingsInstance;
}
