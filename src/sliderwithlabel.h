/*
 * sliderwithlabel.h
 *
 * Copyright 2002-2014 Vesa Halttunen
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
