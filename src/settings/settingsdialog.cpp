#include "settingsdialog.h"
#include "plugins/settingswidget.h"
#include "ui_settingsdialog.h"

#include <QGuiApplication>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QFont bold;
    bold.setBold(true);

    ui->label->setFont(bold);

    connect(ui->listWidget, &QListWidget::currentTextChanged, this, &SettingsDialog::changeCategory);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::onClicked);
    connect(ui->lineEdit, &QLineEdit::textEdited, this, &SettingsDialog::search);

    if (!m_categories.empty())
        ui->listWidget->setCurrentRow(0);
}

void SettingsDialog::changeCategory(const QString &text)
{
    ui->scrollArea->takeWidget();

    QTabWidget *tab = m_categories.value(text);
    if (!tab) {
        ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
        return;
    }

    ui->scrollArea->setWidget(tab);

    ui->label->setText(text);
}

void SettingsDialog::addSettingsWidget(SettingsWidget *w)
{
    if (!w)
        return;

    w->setParent(this);

    auto *tab = m_categories.value(w->displayCategory(), nullptr);
    if (tab) {
        tab->addTab(w, w->displayName());
        return;
    }

    tab = new QTabWidget(this);
    tab->addTab(w, w->displayName());

    m_categories[w->displayCategory()] = tab;

    auto *item = new QListWidgetItem(ui->listWidget);
    item->setText(w->displayCategory());
    item->setIcon(w->categoryIcon());

    ui->listWidget->addItem(item);

    m_settingsWidgets.append(w);

    if (ui->scrollArea->widget() == ui->scrollAreaWidgetContents)
        ui->listWidget->setCurrentRow(0);
}

void SettingsDialog::onClicked(QAbstractButton *button)
{
    switch (ui->buttonBox->buttonRole(button)) {
    case QDialogButtonBox::InvalidRole:
        return;
    case QDialogButtonBox::AcceptRole:
        for (auto *settingsWidget : std::as_const(m_settingsWidgets))
            settingsWidget->finish();
        break;
    case QDialogButtonBox::ApplyRole:
        for (auto *settingsWidget : std::as_const(m_settingsWidgets))
            settingsWidget->apply();
        break;
    default:
        break;
    }
}

void SettingsDialog::search(const QString &text)
{
    int pageCount = 0;

    for (SettingsWidget *settingsWidget : std::as_const(m_settingsWidgets)) {
        bool keywordsContainsText = false;

        for (const QString &keyword : settingsWidget->keyWords()) {
            if (!keyword.contains(text, Qt::CaseInsensitive))
                continue;

            ++pageCount;
            keywordsContainsText = true;
            break;
        }

        auto list = ui->listWidget->findItems(settingsWidget->displayCategory(), Qt::MatchExactly);
        if (list.isEmpty())
            return;

        list.at(0)->setHidden(!keywordsContainsText);
    }

    if (pageCount != 0) {
        if (ui->listWidget->currentItem())
            changeCategory(ui->listWidget->currentItem()->text());
        return;
    }

    ui->scrollArea->takeWidget();
    ui->scrollArea->setWidget(ui->scrollAreaWidgetContents);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
