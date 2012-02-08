#ifndef INSTRUMENTSPINBOX_H
#define INSTRUMENTSPINBOX_H

#include <QSpinBox>

class InstrumentSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit InstrumentSpinBox(QWidget *parent = 0);

protected:
    virtual QString textFromValue(int value) const;
    virtual int valueFromText(const QString &text) const;
};

#endif // INSTRUMENTSPINBOX_H
