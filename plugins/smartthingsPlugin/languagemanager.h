#pragma once

#include <QJsonObject>
#include <QString>

class LanguageManager : public QObject
{
    Q_OBJECT

public:
    explicit LanguageManager(QObject *parent = nullptr);

    void loadLanguageCommands();
    void setLanguage(const QString &language);
    QJsonObject getCurrentCommands() const;
    QString getErrorMessage(const QString &identifier) const;

private:
    QJsonObject m_languageCommands;
    QString m_currentLanguage;
};
