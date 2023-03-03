#include "settingsdialog.h"
#include "plugins/settingswidget.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QFont bold;
    bold.setBold(true);

    ui->label->setFont(bold);

    connect(ui->listWidget, &QListWidget::currentTextChanged, this, &SettingsDialog::changeCategory);

    if (!categories.empty())
        changeCategory(ui->listWidget->item(0)->text());
}

void SettingsDialog::changeCategory(const QString &text)
{
    ui->scrollArea->takeWidget();

    QWidget *widget = categories.value(text);
    if (!widget) {
        ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
        return;
    }

    ui->scrollArea->setWidget(widget);

    ui->label->setText(text);
}

void SettingsDialog::addCatecory(const QString &catecory, const QIcon &icon, SettingsWidget *w)
{
    auto *item = new QListWidgetItem(icon, catecory, ui->listWidget);

    categories[catecory] = w;
    ui->listWidget->addItem(item);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
