#pragma once

#include <QDialog>
#include <QHash>

namespace Ui {
class SettingsDialog;
}
class SettingsWidget;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void addCatecory(const QString &, const QIcon &icon, SettingsWidget *);

private Q_SLOTS:
    void changeCategory(const QString &);

private:
    Ui::SettingsDialog *ui;

    QHash<QString, SettingsWidget *> categories;
};
