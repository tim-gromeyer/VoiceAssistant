#pragma once

#include <QAction>
#include <QListWidget>

class ListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ListWidget(QWidget *parent = nullptr)
        : QListWidget(parent)
        , actionRemove(new QAction(this))
    {
        actionRemove->setText(tr("Remove"));
        actionRemove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
        actionRemove->setShortcuts(QKeySequence::Delete);
        setContextMenuPolicy(Qt::ActionsContextMenu);
        addAction(actionRemove);

        connect(actionRemove, &QAction::triggered, this, [this] {
            const auto selected = selectedItems();

            for (auto *item : selected) {
                QString itemText = item->text();

                items.removeOne(itemText);
                delete item;
                Q_EMIT itemDeleted(itemText);
            }
        });
    };

    [[nodiscard]] inline bool isEmpty() const { return items.isEmpty(); };
    inline void add(const QString &item)
    {
        if (items.contains(item))
            return;

        addItem(item);
        items.append(item);
    };
    [[nodiscard]] inline bool containsItem(const QString &item) const
    {
        return items.contains(item);
    };

    [[nodiscard]] inline const QStringList &allItems() const { return items; };

public Q_SLOTS:
    inline void clear()
    {
        QListWidget::clear();
        items.clear();
    };

Q_SIGNALS:
    void itemDeleted(const QString &item);

private:
    QAction *actionRemove = nullptr;

    QStringList items;
};
