#pragma once

#if !defined(QT_WIDGETS_LIB)
#error Qt Widgets required!
#endif

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class QSettings;

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget *parent = nullptr)
        : QWidget(parent){};
    ~SettingsWidget() = default;

    virtual void apply() = 0;  // Apply the changes (when the apply button is pressed)
    virtual void finish() = 0; // Write settings

    [[nodiscard]] QString displayName() const { return m_displayName; }         // Tab name
    [[nodiscard]] QString displayCategory() const { return m_displayCategory; } // Name in the list
    [[nodiscard]] const QIcon &categoryIcon() const { return m_categoryIcon; }; // Icon in the list

    [[nodiscard]] inline const QStringList &keyWords() // Keywords for the search
    {
        if (m_keywordsInitialized)
            return m_keywords;

        // find common sub-widgets
        for (const QLabel *label : findChildren<QLabel *>())
            m_keywords << label->text();
        for (const QCheckBox *checkbox : findChildren<QCheckBox *>())
            m_keywords << checkbox->text();
        for (const QPushButton *pushButton : findChildren<QPushButton *>())
            m_keywords << pushButton->text();
        for (const QGroupBox *groupBox : findChildren<QGroupBox *>())
            m_keywords << groupBox->title();

        m_keywordsInitialized = true;

        return m_keywords;
    };

    virtual void setSettings(QSettings *settings) final { m_settings = settings; };

protected:
    void setDisplayName(const QString &displayName) { m_displayName = displayName; }
    void setDisplayCategory(const QString &displayCategory) { m_displayCategory = displayCategory; }
    void setCategoryIcon(const QIcon &categoryIcon) { m_categoryIcon = categoryIcon; }
    QSettings *settings() { return m_settings; };

private:
    bool m_keywordsInitialized = false;
    QStringList m_keywords;

    QString m_displayName;
    QString m_displayCategory;
    QIcon m_categoryIcon;

    QSettings *m_settings = nullptr;
};
