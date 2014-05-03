/*
 * songpropertiesdialog.cpp
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

#include "song.h"
#include "songpropertiesdialog.h"
#include "ui_songpropertiesdialog.h"

SongPropertiesDialog::SongPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SongPropertiesDialog),
    song(NULL)
{
    ui->setupUi(this);
}

SongPropertiesDialog::~SongPropertiesDialog()
{
    delete ui;
}

void SongPropertiesDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void SongPropertiesDialog::setSong(Song *song)
{
    if (this->song != song) {
        if (this->song != NULL) {
            disconnect(ui->lineEditName, SIGNAL(textChanged(QString)), this->song, SLOT(setName(QString)));
            disconnect(ui->horizontalSliderTempo, SIGNAL(valueChanged(int)), this->song, SLOT(setTempo(int)));
            disconnect(ui->horizontalSliderTicksPerLine, SIGNAL(valueChanged(int)), this->song, SLOT(setTPL(int)));
        }

        this->song = song;

        ui->lineEditName->setText(song->name());
        ui->horizontalSliderTempo->setValue(song->tempo());
        ui->horizontalSliderTicksPerLine->setValue(song->ticksPerLine());

        connect(ui->lineEditName, SIGNAL(textChanged(QString)), song, SLOT(setName(QString)));
        connect(ui->horizontalSliderTempo, SIGNAL(valueChanged(int)), song, SLOT(setTempo(int)));
        connect(ui->horizontalSliderTicksPerLine, SIGNAL(valueChanged(int)), song, SLOT(setTPL(int)));
    }
}
