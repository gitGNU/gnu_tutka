#ifndef SLIDERWITHLABEL_H
#define SLIDERWITHLABEL_H

#include <QSlider>

class QLabel;

class SliderWithLabel : public QSlider
{
    Q_OBJECT

public:
    explicit SliderWithLabel(QWidget *parent = 0);
    virtual QSize sizeHint() const;

protected:
    virtual void resizeEvent(QResizeEvent *event);

private slots:
    void updateLabel(int value);

private:
    QLabel *label;
};

#endif // SLIDERWITHLABEL_H
