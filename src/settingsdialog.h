#pragma once

#include <QDialog>
#include <QHash>

namespace Ui {
class SettingsDialog;
}
class QAbstractButton;
class QTabWidget;
class SettingsWidget;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void addSettingsWidget(SettingsWidget *w);

private Q_SLOTS:
    void changeCategory(const QString &);

    void onClicked(QAbstractButton *);

    void search(const QString &);

private:
    Ui::SettingsDialog *ui;

    QList<SettingsWidget *> m_settingsWidgets;
    QHash<QString, QTabWidget *> m_categories;
};
