#pragma once

#include <QString>

struct Plugin
{
    bool isValid(const QString &) const { return false; };

    void run(const QString &) const {};
};

Q_DECLARE_TYPEINFO(Plugin, Q_PRIMITIVE_TYPE);
