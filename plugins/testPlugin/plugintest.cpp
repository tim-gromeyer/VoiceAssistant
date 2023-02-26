#include "plugintest.h"

bool PluginTest::isValid(const QString &text)
{
    return text == QLatin1String("test plug in") || text == QLatin1String("plugin testen");
}

void PluginTest::run(const QString & /* text */)
{
    bridge->sayAndWait(tr("Hello"));
    bridge->ask(tr("How are you"));
    bridge->say(tr("Okay"));
}
