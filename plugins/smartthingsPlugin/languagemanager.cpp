#include "languagemanager.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , m_currentLanguage(QLocale::system().name().left(2))
{}

void LanguageManager::loadLanguageCommands()
{
    QFile file(":/language_commands.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open language commands file";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid language commands JSON format";
        return;
    }

    m_languageCommands = doc.object();
    qDebug() << "Loaded language commands for" << m_languageCommands.keys().size() << "languages";
}

void LanguageManager::setLanguage(const QString &language)
{
    if (m_languageCommands.contains(language)) {
        m_currentLanguage = language;
        qDebug() << "Language set to:" << language;
    } else {
        qWarning() << "Unsupported language:" << language << "- falling back to English";
        m_currentLanguage = "en";
    }
}

QJsonObject LanguageManager::getCurrentCommands() const
{
    return m_languageCommands.value(m_currentLanguage).toObject();
}

QString LanguageManager::getErrorMessage(const QString &identifier) const
{
    return getCurrentCommands().value("errors").toObject().value(identifier).toString();
}
