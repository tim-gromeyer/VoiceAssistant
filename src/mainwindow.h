#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

#include "recognizer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QTimer;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void onStateChanged(Recognizer::State);

    void updateTime();

    void onWakeWord();
    void doneListening();

    void updateText(const QString &text);

    void onHasWord();

private:
    Ui::MainWindow *ui;

    QTimer *timeTimer;

    Recognizer *recognizer;
};
#endif // MAINWINDOW_H
