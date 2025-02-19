#include "smartthingssettingswidget.h"
#include "qsettings.h"
#include <QCoreApplication>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

SmartThingsSettingsWidget::SmartThingsSettingsWidget(QWidget *parent)
    : SettingsWidget(parent)
{
    setDisplayName("SmartThings");
    setDisplayCategory("Smart Home");
    setCategoryIcon(QIcon(":/icons/smarthome.png")); // Assuming you have an icon for smart home

    auto *layout = new QVBoxLayout(this);

    auto *formLayout = new QFormLayout();
    layout->addLayout(formLayout);

    clientIdEdit = new QLineEdit(this);
    formLayout->addRow("Client ID:", clientIdEdit);

    clientSecretEdit = new QLineEdit(this);
    clientSecretEdit->setEchoMode(QLineEdit::Password); // Hide the secret
    formLayout->addRow("Client Secret:", clientSecretEdit);

    // Add some padding at the bottom
    layout->addStretch(1);
}

QString SmartThingsSettingsWidget::getAuthorizationToken()
{
    return settings()->value("SmartThings/AuthorizationToken", "").toString();
}

void SmartThingsSettingsWidget::setup()
{
    clientSecretEdit->setText(getAuthorizationToken());
}

void SmartThingsSettingsWidget::apply()
{
    // Apply changes to the current session or memory if needed
}

void SmartThingsSettingsWidget::finish()
{
    // Save the settings
    settings()->setValue("SmartThings/AuthorizationToken", clientSecretEdit->text());

    // Optionally, you might want to signal that settings have changed if this affects runtime behavior
    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
}
