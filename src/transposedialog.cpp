/*
 * transposedialog.cpp
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

#include "song.h"
#include "transposedialog.h"
#include "ui_transposedialog.h"

TransposeDialog::TransposeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransposeDialog),
    song(NULL),
    block(0),
    track(0),
    instrument(0),
    selectionStartTrack(-1),
    selectionStartLine(-1),
    selectionEndTrack(-1),
    selectionEndLine(-1)
{
    ui->setupUi(this);

    connect(ui->pushButtonTranspose, SIGNAL(clicked()), this, SLOT(transpose()));
}

TransposeDialog::~TransposeDialog()
{
    delete ui;
}

void TransposeDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void TransposeDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void TransposeDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void TransposeDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}

void TransposeDialog::setSong(Song *song)
{
    this->song = song;
}

void TransposeDialog::setBlock(unsigned int block)
{
    this->block = block;
}

void TransposeDialog::setTrack(int track)
{
    this->track = track;
}

void TransposeDialog::setInstrument(int instrument)
{
    this->instrument = instrument;
}

void TransposeDialog::setSelection(int startTrack, int startLine, int endTrack, int endLine)
{
    selectionStartTrack = startTrack;
    selectionStartLine = startLine;
    selectionEndTrack = endTrack;
    selectionEndLine = endLine;
}

void TransposeDialog::transpose()
{
    int halfNotes = 0;

    switch (ui->comboBoxMode->currentIndex()) {
    case 0:
        halfNotes = 12;
        break;
    case 1:
        halfNotes = 1;
        break;
    case 2:
        halfNotes = -1;
        break;
    case 3:
        halfNotes = -12;
        break;
    default:
        break;
    }

    int instrument = ui->comboBoxInstruments->currentIndex() == 1 ? this->instrument : -1;

    switch (ui->comboBoxArea->currentIndex()) {
    case SONG:
        song->transpose(instrument, halfNotes);
        break;
    case BLOCK: {
        Block *block = song->block(this->block);
        block->transpose(instrument, halfNotes, 0, 0, block->tracks() - 1, block->length() - 1);
        break;
    }
    case TRACK: {
        Block *block = song->block(this->block);
        block->transpose(instrument, halfNotes, track, 0, track, block->length() - 1);
        break;
    }
    case SELECTION:
        if (selectionStartTrack >= 0 && selectionStartLine >= 0 && selectionEndTrack >= 0 && selectionEndLine >= 0) {
            Block *block = song->block(this->block);
            block->transpose(instrument, halfNotes, selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
        }
        break;
    default:
        break;
    }
}
