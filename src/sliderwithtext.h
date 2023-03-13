#pragma once

#include <QSlider>

class QLabel;

class SliderWithText : public QSlider
{
    Q_OBJECT
public:
    SliderWithText(QWidget *parent = nullptr);

private:
    void resizeEvent(QResizeEvent *);

    Q_SLOT void updateLabel(int);
    QLabel *m_label = nullptr;
};

