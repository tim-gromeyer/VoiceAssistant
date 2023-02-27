#include "plugintest.h"

#include <QLabel>
#include <QVBoxLayout>

bool PluginTest::isValid(const QString &text)
{
    return text == QLatin1String("test plug in") || text == QLatin1String("plugin testen");
}

void PluginTest::run(const QString & /* text */)
{
    bridge->sayAndWait(tr("Hello"));
    bridge->ask(tr("How are you"));
    bridge->say(tr("Okay"));

    auto *w = new QWidget(this);
    auto *l = new QVBoxLayout(w);
    auto *label = new QLabel(tr("It works!"), w);
    l->addWidget(label, 1, Qt::AlignCenter);
    Q_EMIT bridge->useWidget(w);
}
