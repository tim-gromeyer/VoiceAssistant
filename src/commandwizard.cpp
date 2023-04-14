#include "commandwizard.h"
#include "listwidget.h"
#include "mainwindow.h"
#include "plugins/bridge.h"

#include <QAction>
#include <QCommandLinkButton>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>

#include <QDebug>

using namespace CommandWizardNS;

ListWidgetPage::ListWidgetPage(QWidget *parent)
    : QWizardPage(parent)
    , listWidget(new ListWidget)
{
    auto *layout = new QHBoxLayout(this);
    layout->addWidget(listWidget);
};

void ListWidgetPage::add(const QString &item)
{
    listWidget->add(item);
}

QStringList ListWidgetPage::items()
{
    return listWidget->allItems();
}

WelcomePage::WelcomePage(QWidget *parent)
    : QWizardPage(parent)
    , verticalLayout(new QVBoxLayout(this))
    , label(new QLabel(this))
    , spacer1(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding))
    , spacer2(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding))
{
    label->setAlignment(Qt::AlignCenter);

    verticalLayout->addItem(spacer1);
    verticalLayout->addWidget(label);
    verticalLayout->addItem(spacer2);

    setTitle(tr("Welcome to Command Wizard"));

    label->setText(tr("This wizard allows you to add commands.\nPress 'Next' to continue."));

    setLayout(verticalLayout);
}

AddCommandPage::AddCommandPage(PluginBridge *b, QWidget *parent)
    : QWizardPage(parent)
    , bridge(b)
    , verticalLayout(new QVBoxLayout(this))
    , horizontalLayout(new QHBoxLayout())
    , listenButton(new QPushButton(this))
    , commandLabel(new QLabel(this))
    , commandsList(new ListWidget(this))
{
    horizontalLayout->addWidget(commandLabel);

    horizontalLayout->addWidget(listenButton);

    verticalLayout->addLayout(horizontalLayout);

    verticalLayout->addWidget(commandsList);

    registerField(QStringLiteral("commands"), commandsList);

    connect(commandsList, &ListWidget::itemDeleted, this, [this] { Q_EMIT completeChanged(); });
}

void AddCommandPage::initializePage()
{
    setTitle(tr("Add command"));
    setSubTitle(tr("Tap 'Listen' to turn on voice recognition. Speak naturally and the app will "
                   "convert your spoken words into text."));
    listenButton->setText(tr("Listen"));

    connect(listenButton,
            &QPushButton::clicked,
            this,
            &AddCommandPage::addCommand,
            Qt::UniqueConnection);
}

void AddCommandPage::addCommand()
{
    listenButton->setDisabled(true);
    Q_EMIT completeChanged();

    QString text = bridge->ask(QLatin1String());
    if (!text.isEmpty())
        commandsList->add(text);

    listenButton->setDisabled(false);

    Q_EMIT completeChanged();
}

bool AddCommandPage::isComplete() const
{
    return !commandsList->isEmpty() && listenButton->isEnabled();
}

ActionPage::ActionPage(ListWidgetPage *page, QWidget *parent)
    : QWizardPage(parent)
    , listWidgetPage(page)
    , verticalLayout(new QVBoxLayout(this))
    , groupBox(new QGroupBox(this))
    , formLayout(new QFormLayout(groupBox))
    , executeLabel(new QLabel(groupBox))
    , functionEdit(new QLineEdit(groupBox))
    , randomLabel(new QLabel(groupBox))
    , randomResponseButton(new QPushButton(groupBox))
    , programmLabel(new QLabel(groupBox))
    , horizontalLayout(new QHBoxLayout())
    , programEdit(new QLineEdit(groupBox))
    , selectProgrammButton(new QToolButton(groupBox))
    , argumentLabel(new QLabel(groupBox))
    , argumentButton(new QPushButton(groupBox))
    , playLabel(new QLabel(groupBox))
    , soundEdit(new QLineEdit(groupBox))
{
    formLayout->setWidget(0, QFormLayout::LabelRole, executeLabel);
    formLayout->setWidget(0, QFormLayout::FieldRole, functionEdit);

    formLayout->setWidget(1, QFormLayout::LabelRole, randomLabel);
    formLayout->setWidget(1, QFormLayout::FieldRole, randomResponseButton);

    formLayout->setWidget(2, QFormLayout::LabelRole, playLabel);
    formLayout->setWidget(2, QFormLayout::FieldRole, soundEdit);

    horizontalLayout->addWidget(programEdit);

    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(selectProgrammButton->sizePolicy().hasHeightForWidth());
    selectProgrammButton->setSizePolicy(sizePolicy);

    horizontalLayout->addWidget(selectProgrammButton);

    horizontalLayout->setStretch(0, 1);

    formLayout->setWidget(3, QFormLayout::LabelRole, programmLabel);
    formLayout->setLayout(3, QFormLayout::FieldRole, horizontalLayout);

    formLayout->setWidget(4, QFormLayout::LabelRole, argumentLabel);
    formLayout->setWidget(4, QFormLayout::FieldRole, argumentButton);

    verticalLayout->addWidget(groupBox);

    setTitle(tr("Select Action"));
    setSubTitle(tr("Select the action(s) to be performed when the command is recognized"));
    groupBox->setTitle(tr("Actions"));
    executeLabel->setText(tr("Execute function"));
    randomLabel->setText(tr("Random response"));
    randomResponseButton->setText(tr("Select"));
    programmLabel->setText(tr("Execute program"));
    selectProgrammButton->setText(QStringLiteral("..."));
    argumentLabel->setText(tr("Program arguments"));
    argumentButton->setText(tr("Select"));
    playLabel->setText(tr("Play a sound"));
    soundEdit->setPlaceholderText(tr("URLs are also supported"));

    connect(functionEdit, &QLineEdit::textEdited, this, &ActionPage::checkFunctionExists);
    connect(programEdit, &QLineEdit::textChanged, this, &ActionPage::checkAppPath);
    connect(selectProgrammButton, &QToolButton::clicked, this, &ActionPage::selectAppPath);
}

bool ActionPage::checkFunctionExists(const QString &funcName)
{
    QMetaObject object = MainWindow::staticMetaObject;

    int index = object.indexOfMethod(QString(funcName + "()").toUtf8());

    if (index == -1) {
        functionEdit->setStyleSheet(QStringLiteral("color: red"));
        return false;
    } else {
        functionEdit->setStyleSheet(QStringLiteral("color: green"));
        return true;
    }
}

bool ActionPage::checkAppPath(const QString &text)
{
    if (QFile::exists(text) || text.isEmpty()) {
        programEdit->setStyleSheet(QStringLiteral("color: green"));
        return true;
    } else {
        programEdit->setStyleSheet(QStringLiteral("color: red"));
        return false;
    }
}

void ActionPage::selectAppPath()
{
    QFileDialog dia(this, tr("Select executable"));
    dia.setAcceptMode(QFileDialog::AcceptOpen);
    dia.setMimeTypeFilters({QStringLiteral("application/x-executable")});
#ifdef Q_OS_LINUX
    // https://github.com/KDAB/hotspot/issues/286
    dia.setOption(QFileDialog::DontUseNativeDialog);
#endif
    if (!dia.exec())
        return;

    auto files = dia.selectedFiles();
    if (files.isEmpty())
        return;

    programEdit->setText(files.at(0));
}
void ActionPage::selectAppArgs() {}
void ActionPage::selectRandomResponses() {}

CommandWizard::CommandWizard(PluginBridge *b, QWidget *parent)
    : QWizard(parent)
    , bridge(b)
    , listWidgetPage(new ListWidgetPage(this))
    , welcomePage(new WelcomePage(this))
    , addCommandPage(new AddCommandPage(bridge, this))
    , actionPage(new ActionPage(listWidgetPage, this))
{
    setWindowTitle(tr("Add command"));

    setPage(Page_Welcome, welcomePage);
    setPage(Page_AddCommand, addCommandPage);
    setPage(Page_Action, actionPage);
    setPage(Page_ListWidget, listWidgetPage);

    connect(actionPage, &ActionPage::gotoListWidgetPage, this, [this] {
        // Note: Untested
        setProperty("currentId", Page_ListWidget);
        Q_EMIT currentIdChanged(Page_ListWidget);
    });
}
