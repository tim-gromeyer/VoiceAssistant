#ifndef COMMANDWIZARD_H
#define COMMANDWIZARD_H

#include "utils.h"

#include <QWizard>

class ListWidget;
class PluginBridge;
class QCommandLinkButton;
class QFormLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpacerItem;
class QToolButton;
class QVBoxLayout;
class QWizardPage;

namespace CommandWizardNS {
enum Pages { Page_Welcome = 0, Page_AddCommand = 1, Page_Action = 2, Page_ListWidget = 3 };

} // namespace CommandWizardNS

class ListWidgetPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ListWidgetPage(QWidget *parent = nullptr);

    void add(const QString &);

    QStringList items();

    [[nodiscard]] inline int nextId() const override { return CommandWizardNS::Page_AddCommand; };

private:
    ListWidget *listWidget;
};

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

    void initializePage() override;
    [[nodiscard]] bool isComplete() const override;

    [[nodiscard]] QStringList commands() const;

private Q_SLOTS:
    void addCommand();

private:
    PluginBridge *bridge;

    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *listenButton;
    QLabel *commandLabel;
    ListWidget *commandsList;
};

class ActionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ActionPage(QWidget *parent = nullptr);

    actions::Action getAction(bool *valid);

    [[nodiscard]] inline int nextId() const override { return -1; };

private Q_SLOTS:
    bool checkFunctionExists(const QString &funcName);

    void selectAppPath();
    void selectAppArgs();
    bool checkAppPath(const QString &text);

    void selectRandomResponses();

private:
    enum Mode { Nothing = -1, RandomResponse = 0, AppArguments = 1 };

    Mode currentMode = Nothing;

    actions::Action action;

    QWidget *contentWidget;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QFormLayout *formLayout;
    QLabel *actionNameLabel;
    QLineEdit *actionNameLineEdit;
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

    QPushButton *addButton;
    ListWidget *listWidget;
    QPushButton *okayButton;
};

class CommandWizard : public QWizard
{
    Q_OBJECT

public:
    explicit CommandWizard(PluginBridge *b, QWidget *parent = nullptr);
    ~CommandWizard() = default;

    actions::Action getAction(bool *valid);

private:
    PluginBridge *bridge;

    WelcomePage *welcomePage;
    AddCommandPage *addCommandPage;
    ActionPage *actionPage;
};

#endif // ADDCOMMANDYISYFT_H
