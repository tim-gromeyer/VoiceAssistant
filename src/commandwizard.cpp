#include "commandwizard.h"
#include "plugins/bridge.h"

#include <QCommandLinkButton>
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
    , commandsList(new QListWidget(this))
{
    horizontalLayout->addWidget(commandLabel);

    horizontalLayout->addWidget(listenButton);

    verticalLayout->addLayout(horizontalLayout);

    verticalLayout->addWidget(commandsList);

    setTitle(tr("Add command"));
    setSubTitle(tr("Tap 'Listen' to turn on voice recognition. Speak naturally and the app will "
                   "convert your spoken words into text."));
    listenButton->setText(tr("Add command"));

    connect(listenButton, &QPushButton::clicked, this, &AddCommandPage::addCommand);
}

void AddCommandPage::addCommand()
{
    listenButton->setDisabled(true);
    QString text = bridge->ask(QLatin1String());
    qDebug() << text;
    commandsList->addItem(text);
    listenButton->setDisabled(false);
}

ActionPage::ActionPage(QWidget *parent)
    : QWizardPage(parent)
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
    argumentLabel->setText(tr("Programm arguments"));
    argumentButton->setText(tr("Select"));
    playLabel->setText(tr("Play a sound"));
    soundEdit->setPlaceholderText(tr("URL's are also supported"));
}

CommandWizard::CommandWizard(PluginBridge *b, QWidget *parent)
    : QWizard(parent)
    , bridge(b)
    , welcomePage(new WelcomePage(this))
    , addCommandPage(new AddCommandPage(bridge, this))
    , actionPage(new ActionPage(this))
{
    addPage(welcomePage);
    addPage(addCommandPage);
    addPage(actionPage);
}
