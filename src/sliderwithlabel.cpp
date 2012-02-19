#include <QLabel>
#include "sliderwithlabel.h"

SliderWithLabel::SliderWithLabel(QWidget *parent) :
    QSlider(parent),
    label(new QLabel(this))
{
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateLabel(int)));
}

void SliderWithLabel::updateLabel(int value)
{
    label->setText(QString("%1").arg(value));

    int range = maximum() - minimum();
    int labelWidth = label->sizeHint().width();
    int usableWidth = width() - labelWidth;
    label->setGeometry((value - minimum()) * usableWidth / range, 0, labelWidth, label->sizeHint().height());
}

QSize SliderWithLabel::sizeHint() const
{
    return QSlider::sizeHint() + QSize(0, label->sizeHint().height() + 15);
}

void SliderWithLabel::resizeEvent(QResizeEvent *event)
{
    QSlider::resizeEvent(event);

    updateLabel(value());
}
