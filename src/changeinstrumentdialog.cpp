/*
 * changeinstrumentdialog.cpp
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
#include "changeinstrumentdialog.h"
#include "ui_changeinstrumentdialog.h"

ChangeInstrumentDialog::ChangeInstrumentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeInstrumentDialog),
    song(NULL),
    block(0),
    track(0),
    selectionStartTrack(-1),
    selectionStartLine(-1),
    selectionEndTrack(-1),
    selectionEndLine(-1)
{
    ui->setupUi(this);

    connect(ui->pushButtonSwap, SIGNAL(clicked()), this, SLOT(swap()));
    connect(ui->pushButtonChange, SIGNAL(clicked()), this, SLOT(change()));
}

ChangeInstrumentDialog::~ChangeInstrumentDialog()
{
    delete ui;
}

void ChangeInstrumentDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void ChangeInstrumentDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void ChangeInstrumentDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void ChangeInstrumentDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}


void ChangeInstrumentDialog::setSong(Song *song)
{
    this->song = song;
}

void ChangeInstrumentDialog::setBlock(unsigned int block)
{
    this->block = block;
}

void ChangeInstrumentDialog::setTrack(int track)
{
    this->track = track;
}

void ChangeInstrumentDialog::setSelection(int startTrack, int startLine, int endTrack, int endLine)
{
    selectionStartTrack = startTrack;
    selectionStartLine = startLine;
    selectionEndTrack = endTrack;
    selectionEndLine = endLine;
}

void ChangeInstrumentDialog::swap()
{
    change(true);
}

void ChangeInstrumentDialog::change(bool swap)
{
    int from = ui->spinBoxFrom->value();
    int to = ui->spinBoxTo->value();

    switch (ui->comboBoxArea->currentIndex()) {
    case AreaSong:
        song->changeInstrument(from, to, swap);
        break;
    case AreaBlock: {
        Block *block = song->block(this->block);
        block->changeInstrument(from, to, swap, 0, 0, block->tracks() - 1, block->length() - 1);
        break;
    }
    case AreaTrack: {
        Block *block = song->block(this->block);
        block->changeInstrument(from, to, swap, track, 0, track, block->length() - 1);
        break;
    }
    case AreaSelection:
        if (selectionStartTrack >= 0 && selectionStartLine >= 0 && selectionEndTrack >= 0 && selectionEndLine >= 0) {
            Block *block = song->block(this->block);
            block->changeInstrument(from, to, swap, selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
        }
        break;
    default:
        break;
    }

    if (swap) {
        song->checkInstrument(from);
    }
    song->checkInstrument(to);
}
