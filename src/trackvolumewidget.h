/*
 * trackvolumewidget.h
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

#ifndef TRACKVOLUMEWIDGET_H
#define TRACKVOLUMEWIDGET_H

#include <QWidget>

class Track;
class QLabel;
class QSlider;
class QCheckBox;
class QLineEdit;

class TrackVolumeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrackVolumeWidget(unsigned int number, Track *track, QWidget *parent = 0);

private slots:
    void updateVolumeLabel(int volume);

private:
    unsigned int trackNumber;
    Track *track;
    QLabel *volumeLabel;
    QSlider *volumeSlider;
    QCheckBox *muteCheckBox;
    QCheckBox *soloCheckBox;
    QLineEdit *nameLineEdit;
};

#endif // TRACKVOLUMEWIDGET_H
