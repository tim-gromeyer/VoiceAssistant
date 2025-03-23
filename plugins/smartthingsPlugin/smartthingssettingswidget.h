#pragma once

#include "settingswidget.h"
#include <QLineEdit>

class SmartThingsSettingsWidget : public SettingsWidget
{
    Q_OBJECT
public:
    explicit SmartThingsSettingsWidget(QWidget *parent = nullptr);
    ~SmartThingsSettingsWidget() override = default;

    void setup() override;
    void apply() override;
    void finish() override;

    QString getAuthorizationToken();

private:
    QLineEdit *clientIdEdit;
    QLineEdit *clientSecretEdit;
};
