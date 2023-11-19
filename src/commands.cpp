#include "commands.h"
#include "global.h"
#include "mainwindow.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QObject>
#include <QRegularExpression>
#include <QSaveFile>

using utils::strings::literals::L1;

QString getPlatformSpecificExecutable(const QString &exe)
{
    QStringList parts = exe.split(u' ');
    QString platformPrefix = info::getOsString() + ':';
    int index = parts.indexOf(platformPrefix);

    static const QRegularExpression regex(L1("(Windows|MacOS|Linux|WebAssembly):"),
                                          QRegularExpression::CaseInsensitiveOption);

    if (index == -1 && exe.contains(regex))
        return {};

    return (index != -1 && index + 1 < parts.size()) ? parts[index + 1] : exe;
}

QList<actions::Action> Commands::getActions(const QString &language)
{
    QList<actions::Action> commands;

    QString dir = dir::commandsBaseDir() + language;

    // Fallback solution if `firstSetup` failed to copy the folder
    QDir testDir;
    testDir.setPath(dir);
    if (testDir.isEmpty())
        dir = dir::commandsInstallBaseDir() + language;

    QFile jsonFile(dir + STR("/default.json"));
    // open the JSON file
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qCritical() << STR("Failed to open %1:\n%2")
                           .arg(jsonFile.fileName(), jsonFile.errorString())
                           .toStdString()
                           .c_str();
        return {};
    }

    qDebug() << "[debug] Loading commands from" << jsonFile.fileName();

    // read all the data from the JSON file
    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();

    // In case of an error
    QJsonParseError error{};

    // create a QJsonDocument from the JSON data
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);

    // Error handling
    if (error.error != QJsonParseError::NoError) {
        qCritical() << STR("JSON parsing error at %1: %2")
                           .arg(QString::number(error.offset), error.errorString());
        return {};
    }

    // get the array from the JSON document
    QJsonArray jsonArray = jsonDoc.array();

    for (const auto &value : jsonArray) {
        // convert the array element to an object
        QJsonObject jsonObject = value.toObject();

        // Command template
        actions::Action c;

        // get the commands (aka key words) from the JSON object
        QJsonArray commandsArray = jsonObject[STR("commands")].toArray();
        if (commandsArray.isEmpty()) {
            qWarning() << "No commands specified for:" << jsonObject;
            continue;
        }
        c.commands.reserve(commandsArray.size());
        for (const auto &command : commandsArray)
            c.commands << command.toString();

        // get the name of the action
        c.name = jsonObject[STR("name")].toString();

        // this is a default action
        c.isUserAction = false;

        // get the function name from the JSON object(optional)
        c.funcName = jsonObject[STR("funcName")].toString();

        // get the responses(optional)
        QJsonArray responseArray = jsonObject[STR("responses")].toArray();
        for (const auto &response : responseArray)
            c.responses.append(response.toString());

        // sound/song to play(optional)
        c.sound = jsonObject[STR("sound")].toString();

        // used to execute external programs(optional)
        c.program = jsonObject[STR("program")].toString();
        c.program = getPlatformSpecificExecutable(c.program);
        QJsonArray args = jsonObject[STR("args")].toArray();
        c.args.reserve(args.size());
        for (const auto &arg : args)
            c.args.append(getPlatformSpecificExecutable(arg.toString()));

        commands.append(c);
    }

    qDebug().nospace().noquote() << "[debug] Loaded " << commands.size() << " commands";

    return commands;
}

bool Commands::saveActions(const QList<actions::Action> &commands, const QString &language)
{
    const QString dir = dir::commandsBaseDir() + language;
    QDir makeDir;
    makeDir.setPath(dir);
    makeDir.mkpath(dir);

    QJsonArray jsonArray;
    QJsonArray userArray;

    for (const auto &c : std::as_const(commands)) {
        QJsonObject jsonObject;

        if (!c.name.isEmpty())
            jsonObject[STR("name")] = c.name;
        if (!c.funcName.isEmpty())
            jsonObject[STR("funcName")] = c.funcName;
        if (!c.program.isEmpty())
            jsonObject[STR("program")] = c.program;
        if (!c.sound.isEmpty())
            jsonObject[STR("sound")] = c.sound;

        if (!c.commands.isEmpty()) {
            QJsonArray commandsArray;
            for (const auto &command : c.commands)
                commandsArray.append(command);
            jsonObject[STR("commands")] = commandsArray;
        }

        if (!c.responses.isEmpty()) {
            QJsonArray responseArray;
            for (const auto &response : c.responses)
                responseArray.append(response);
            jsonObject[STR("responses")] = responseArray;
        }

        if (!c.args.isEmpty()) {
            QJsonArray argsArray;
            for (const auto &arg : c.args)
                argsArray.append(arg);
            jsonObject[STR("args")] = argsArray;
        }
        if (!jsonObject.isEmpty()) {
            if (c.isUserAction)
                userArray.append(jsonObject);
            else
                jsonArray.append(jsonObject);
        }
    }

    QJsonDocument jsonDoc(jsonArray);

    QSaveFile jsonFile(dir + STR("/default.json"));
    if (!jsonFile.open(QIODevice::WriteOnly)) {
        qDebug() << STR("Failed to open %1\n%2").arg(jsonFile.fileName(), jsonFile.errorString());
        return false;
    }
    jsonFile.write(jsonDoc.toJson());
    if (!jsonFile.commit()) {
        QMessageBox::warning(
            MainWindow::instance(),
            QObject::tr("Failed to save file"),
            QObject::tr(
                "Failed to write <em>%1</em>.\n%2\nCopy following text and save it manually:\n%3")
                .arg(jsonFile.fileName(), jsonFile.errorString(), jsonDoc.toJson()));
        return false;
    }

    QJsonDocument userJsonDoc(userArray);

    QSaveFile userJsonFile(dir + STR("/user.json"));
    if (!userJsonFile.open(QIODevice::WriteOnly)) {
        qDebug() << STR("Failed to open %1\n%2")
                        .arg(userJsonFile.fileName(), userJsonFile.errorString());
        return false;
    }
    userJsonFile.write(userJsonDoc.toJson());
    if (!userJsonFile.commit()) {
        QMessageBox::warning(
            MainWindow::instance(),
            QObject::tr("Failed to save file"),
            QObject::tr(
                "Failed to write <em>%1</em>.\n%2\nCopy following text and save it manually:\n%3")
                .arg(userJsonFile.fileName(), userJsonFile.errorString(), userJsonDoc.toJson()));
        return false;
    }

    return true;
}
