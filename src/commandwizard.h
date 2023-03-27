#ifndef COMMANDWIZARD_H
#define COMMANDWIZARD_H

#include <QWizard>

class PluginBridge;
class QCommandLinkButton;
class QFormLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpacerItem;
class QToolButton;
class QVBoxLayout;
class QWizardPage;

class WelcomePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);

private:
    QVBoxLayout *verticalLayout;
    QLabel *label;

    QSpacerItem *spacer1;
    QSpacerItem *spacer2;
};

class AddCommandPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit AddCommandPage(PluginBridge *b, QWidget *parent = nullptr);

private Q_SLOTS:
    void addCommand();

private:
    PluginBridge *bridge;
    bool m_asking = false;

    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *listenButton;
    QLabel *commandLabel;
    QListWidget *commandsList;
};

class ActionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ActionPage(QWidget *parent = nullptr);

private:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *executeLabel;
    QLineEdit *functionEdit;
    QLabel *randomLabel;
    QPushButton *randomResponseButton;
    QLabel *programmLabel;
    QHBoxLayout *horizontalLayout;
    QLineEdit *programEdit;
    QToolButton *selectProgrammButton;
    QLabel *argumentLabel;
    QPushButton *argumentButton;
    QLabel *playLabel;
    QLineEdit *soundEdit;
};

class CommandWizard : public QWizard
{
    Q_OBJECT

public:
    explicit CommandWizard(PluginBridge *b, QWidget *parent = nullptr);
    ~CommandWizard() = default;

private:
    PluginBridge *bridge;

    WelcomePage *welcomePage;
    AddCommandPage *addCommandPage;
    ActionPage *actionPage;
};

#endif // ADDCOMMANDYISYFT_H
