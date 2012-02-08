#include "instrumentspinbox.h"

InstrumentSpinBox::InstrumentSpinBox(QWidget *parent) :
    QSpinBox(parent)
{
}

QString InstrumentSpinBox::textFromValue(int value) const
{
    return QString("%1").arg(value + 1, 2, 16, QChar('0'));
}

int InstrumentSpinBox::valueFromText(const QString &text) const
{
    return text.toInt(NULL, 16) - 1;
}
