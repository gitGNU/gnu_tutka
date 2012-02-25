/*
 * instrumentspinbox.cpp
 *
 * Copyright 2002-2012 Vesa Halttunen
 *
 * This file is part of Tutka.
 *
 * Tutka is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "instrumentspinbox.h"

InstrumentSpinBox::InstrumentSpinBox(QWidget *parent) :
    QSpinBox(parent)
{
}

QString InstrumentSpinBox::textFromValue(int value) const
{
    return QString("%1").arg(value + 1, 2, 16, QChar('0')).toUpper();
}

int InstrumentSpinBox::valueFromText(const QString &text) const
{
    return text.toInt(NULL, 16) - 1;
}
