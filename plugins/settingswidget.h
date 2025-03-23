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
public:
    /**
     * @brief SettingsWidget constructor.
     * @param parent The parent widget.
     */
    explicit SettingsWidget(QWidget *parent = nullptr)
        : QWidget(parent){};

    /**
     * @brief SettingsWidget destructor.
     *
     * The destructor is responsible for cleaning up the object.
     */
    ~SettingsWidget() = default;

    /**
     * @brief THe setup function. Do every setup-related here, and not in the constructor. Things like settings will not be set.
     */
    virtual void setup() = 0;

    /**
     * @brief apply applies the changes made in the settings widget.
     *
     * This function is called when the apply button is pressed in the settings widget.
     * Subclasses should implement this function to apply the changes made by the user.
     */
    virtual void apply() = 0;

    /**
     * @brief finish writes the settings to storage.
     *
     * This function is called when the settings are about to be saved.
     * Subclasses should implement this function to write the settings to the appropriate storage.
     */
    virtual void finish() = 0;

    /**
     * @brief displayName returns the display name of the settings widget.
     * @return The display name.
     *
     * This function returns the display name of the settings widget, which is used as the tab name.
     */
    [[nodiscard]] inline QString displayName() const { return m_displayName; }

    /**
     * @brief displayCategory returns the display category of the settings widget.
     * @return The display category.
     *
     * This function returns the display category of the settings widget, which is used as the name in the list.
     */
    [[nodiscard]] inline QString displayCategory() const { return m_displayCategory; }

    /**
     * @brief categoryIcon returns the icon of the settings widget category.
     * @return The category icon.
     *
     * This function returns the icon of the settings widget category, which is displayed in the list.
     */
    [[nodiscard]] inline const QIcon &categoryIcon() const { return m_categoryIcon; }

    /**
     * @brief keyWords returns the keywords associated with the settings widget.
     * @return The list of keywords.
     *
     * This function returns the keywords associated with the settings widget.
     * The keywords are used for searching within the settings.
     */
    [[nodiscard]] inline const QStringList &keyWords()
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

    /**
     * @brief setSettings sets the QSettings object to be used by the settings widget.
     * @param settings The QSettings object.
     *
     * This function sets the QSettings object to be used by the settings widget.
     * The settings object is used for storing and retrieving the settings values.
     */
    virtual void setSettings(QSettings *settings) final { m_settings = settings; };

protected:
    /**
     * @brief setDisplayName sets the display name of the settings widget.
     * @param displayName The display name.
     *
     * This function sets the display name of the settings widget.
     * The display name is used as the tab name for the settings widget.
     */
    inline void setDisplayName(const QString &displayName) { m_displayName = displayName; }

    /**
     * @brief setDisplayCategory sets the display category of the settings widget.
     * @param displayCategory The display category.
     *
     * This function sets the display category of the settings widget.
     * The display category is used as the name in the list for the settings widget.
     */
    inline void setDisplayCategory(const QString &displayCategory)
    {
        m_displayCategory = displayCategory;
    }

    /**
     * @brief setCategoryIcon sets the icon of the settings widget category.
     * @param categoryIcon The category icon.
     *
     * This function sets the icon of the settings widget category.
     * The category icon is displayed in the list for the settings widget.
     */
    inline void setCategoryIcon(const QIcon &categoryIcon) { m_categoryIcon = categoryIcon; }

    /**
     * @brief settings returns the QSettings object used by the settings widget.
     * @return The QSettings object.
     *
     * This function returns the QSettings object used by the settings widget.
     * The settings object is used for storing and retrieving the settings values.
     */
    inline QSettings *settings() { return m_settings; };

private:
    bool m_keywordsInitialized = false;
    QStringList m_keywords;

    QString m_displayName;
    QString m_displayCategory;
    QIcon m_categoryIcon;

    QSettings *m_settings = nullptr;
};
