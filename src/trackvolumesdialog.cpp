/*
 * trackvolumesdialog.cpp
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

#include "song.h"
#include "trackvolumewidget.h"
#include "trackvolumesdialog.h"
#include "ui_trackvolumesdialog.h"

TrackVolumesDialog::TrackVolumesDialog(QWidget *parent) :
    TutkaDialog(parent),
    ui(new Ui::TrackVolumesDialog),
    song(NULL)
{
    ui->setupUi(this);
}

TrackVolumesDialog::~TrackVolumesDialog()
{
    delete ui;
}

void TrackVolumesDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void TrackVolumesDialog::setSong(Song *song)
{
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(maxTracksChanged(uint)), this, SLOT(setTracks(uint)));
    }

    this->song = song;
    setTracks(this->song->maxTracks());

    connect(this->song, SIGNAL(maxTracksChanged(uint)), this, SLOT(setTracks(uint)));
}

void TrackVolumesDialog::setTracks(unsigned int tracks)
{
    while(ui->scrollAreaWidgetContents->layout()->count() > 0) {
        QLayoutItem *item = ui->scrollAreaWidgetContents->layout()->takeAt(0);
        delete item->widget();
        delete item;
    }

    for (int track = 0; track < tracks; track++) {
        ui->scrollAreaWidgetContents->layout()->addWidget(new TrackVolumeWidget(track + 1, song->track(track)));
    }
}
