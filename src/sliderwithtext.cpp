#include "sliderwithtext.h"

#include <QDebug>
#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>

SliderWithText::SliderWithText(QWidget *parent)
    : QSlider(parent)
    , m_label(new QLabel(QStringLiteral("Text"), this))
{
    auto *l = new QVBoxLayout(this);
    l->addWidget(m_label, 0, Qt::AlignCenter);
    setLayout(l);

    m_label->setMinimumSize(30, 20);

    connect(this, &QSlider::valueChanged, this, &SliderWithText::updateLabel);
}

void SliderWithText::updateLabel(int value)
{
    qDebug() << m_label->pos() << m_label->size() << pos() << size();
    m_label->setText(QString::number(value));
}

void SliderWithText::resizeEvent(QResizeEvent *e)
{
    qDebug() << e << e->oldSize() << e->size();
}
