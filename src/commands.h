#ifndef COMMANDS_H
#define COMMANDS_H

#include "utils.h"

#include <QList>
#include <QString>

class Commands
{
public:
    static QList<actions::Action> getActions(const QString &language);
    static bool saveActions(const QList<actions::Action> &list, const QString &language);
};

#endif // COMMANDS_H
