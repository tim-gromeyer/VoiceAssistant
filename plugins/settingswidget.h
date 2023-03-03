#pragma once

#if !defined(QT_WIDGETS_LIB)
#error Qt Widgets required!
#endif

#include <QAbstractButton>
#include <QLabel>
#include <QWidget>

class SettingsWidget : public QWidget
{
    Q_OBJECT
public:
    SettingsWidget() = default;
    ~SettingsWidget() = default;

    bool find(const QString &search)
    {
        const QObjectList &childs = children();
        for (auto *child : childs) {
            auto *label = qobject_cast<QLabel *>(child);
            auto *button = qobject_cast<QAbstractButton *>(child);
            if (label && label->text().contains(search))
                return true;
            if (button && button->text().contains(search))
                return true;
        };

        return true;
    };
};
