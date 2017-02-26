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
        disconnect(this->song, SIGNAL(tracksChanged()), this, SLOT(setTracks()));
    }

    this->song = song;
    setTracks(this->song->maxTracks());
    setTracks();

    connect(this->song, SIGNAL(maxTracksChanged(uint)), this, SLOT(setTracks(uint)));
    connect(this->song, SIGNAL(tracksChanged()), this, SLOT(setTracks()));
}

void TrackVolumesDialog::setTracks(unsigned int tracks)
{
    while(ui->scrollAreaWidgetContents->layout()->count() > tracks) {
        QLayoutItem *item = ui->scrollAreaWidgetContents->layout()->takeAt(ui->scrollAreaWidgetContents->layout()->count() - 1);
        delete item->widget();
        delete item;
    }

    while(ui->scrollAreaWidgetContents->layout()->count() < tracks) {
        ui->scrollAreaWidgetContents->layout()->addWidget(new TrackVolumeWidget(ui->scrollAreaWidgetContents->layout()->count() + 1, song->track(ui->scrollAreaWidgetContents->layout()->count())));
    }
}

void TrackVolumesDialog::setTracks()
{
    for (int track = 0; track < song->maxTracks(); track++) {
        QLayoutItem *item = ui->scrollAreaWidgetContents->layout()->itemAt(track);
        TrackVolumeWidget *widget = qobject_cast<TrackVolumeWidget *>(item->widget());
        widget->setTrack(song->track(track));
    }
}
