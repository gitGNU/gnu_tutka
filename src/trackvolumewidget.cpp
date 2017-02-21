/*
 * trackvolumewidget.cpp
 *
 * Copyright 2002-2016 Vesa Halttunen
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

#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include "track.h"
#include "trackvolumewidget.h"

TrackVolumeWidget::TrackVolumeWidget(unsigned int number, QWidget *parent) :
    QWidget(parent),
    trackNumber(number),
    track(NULL),
    volumeLabel(new QLabel),
    volumeSlider(new QSlider),
    muteCheckBox(new QCheckBox(tr("Mute"))),
    soloCheckBox(new QCheckBox(tr("Solo"))),
    nameLineEdit(new QLineEdit)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Track %1").arg(number)));
    layout->addWidget(volumeLabel);
    layout->addWidget(volumeSlider);
    layout->addWidget(muteCheckBox);
    layout->addWidget(soloCheckBox);
    layout->addWidget(nameLineEdit);

    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(127);
    connect(volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(updateVolumeLabel(int)));
}

void TrackVolumeWidget::setTrack(Track *newTrack)
{
    if (track) {
        disconnect(volumeSlider, SIGNAL(valueChanged(int)), track, SLOT(setVolume(int)));
        disconnect(muteCheckBox, SIGNAL(toggled(bool)), track, SLOT(setMute(bool)));
        disconnect(soloCheckBox, SIGNAL(toggled(bool)), track, SLOT(setSolo(bool)));
        disconnect(track, SIGNAL(volumeChanged(int)), volumeSlider, SLOT(setValue(int)));
        disconnect(track, SIGNAL(mutedChanged(bool)), muteCheckBox, SLOT(setChecked(bool)));
        disconnect(track, SIGNAL(soloChanged(bool)), soloCheckBox, SLOT(setChecked(bool)));
        disconnect(nameLineEdit, SIGNAL(textChanged(QString)), track, SLOT(setName(QString)));
    }

    track = newTrack;

    if (track) {
        volumeSlider->setValue(track->volume());
        updateVolumeLabel(track->volume());
        muteCheckBox->setChecked(track->isMuted());
        soloCheckBox->setChecked(track->isSolo());
        nameLineEdit->setText(track->name());

        connect(volumeSlider, SIGNAL(valueChanged(int)), track, SLOT(setVolume(int)));
        connect(muteCheckBox, SIGNAL(toggled(bool)), track, SLOT(setMute(bool)));
        connect(soloCheckBox, SIGNAL(toggled(bool)), track, SLOT(setSolo(bool)));
        connect(track, SIGNAL(volumeChanged(int)), volumeSlider, SLOT(setValue(int)));
        connect(track, SIGNAL(mutedChanged(bool)), muteCheckBox, SLOT(setChecked(bool)));
        connect(track, SIGNAL(soloChanged(bool)), soloCheckBox, SLOT(setChecked(bool)));
        connect(nameLineEdit, SIGNAL(textChanged(QString)), track, SLOT(setName(QString)));
    }
}

void TrackVolumeWidget::updateVolumeLabel(int volume)
{
    volumeLabel->setText(QString("%1").arg(volume));
}
