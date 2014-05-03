/*
 * expandshrinkdialog.cpp
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
#include "expandshrinkdialog.h"
#include "ui_expandshrinkdialog.h"

ExpandShrinkDialog::ExpandShrinkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExpandShrinkDialog),
    song(NULL),
    block(0),
    track(0),
    selectionStartTrack(-1),
    selectionStartLine(-1),
    selectionEndTrack(-1),
    selectionEndLine(-1)
{
    ui->setupUi(this);

    connect(ui->pushButtonExpand, SIGNAL(clicked()), this, SLOT(expand()));
    connect(ui->pushButtonShrink, SIGNAL(clicked()), this, SLOT(shrink()));
}

ExpandShrinkDialog::~ExpandShrinkDialog()
{
    delete ui;
}

void ExpandShrinkDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void ExpandShrinkDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    makeVisible();
}

void ExpandShrinkDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    makeVisible();
}

void ExpandShrinkDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    makeVisible();
}

void ExpandShrinkDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    makeVisible();
}

void ExpandShrinkDialog::setSong(Song *song)
{
    this->song = song;
}

void ExpandShrinkDialog::setBlock(unsigned int block)
{
    this->block = block;
}

void ExpandShrinkDialog::setTrack(int track)
{
    this->track = track;
}

void ExpandShrinkDialog::setSelection(int startTrack, int startLine, int endTrack, int endLine)
{
    selectionStartTrack = startTrack;
    selectionStartLine = startLine;
    selectionEndTrack = endTrack;
    selectionEndLine = endLine;
}

void ExpandShrinkDialog::expand()
{
    expandShrink(false);
}

void ExpandShrinkDialog::shrink()
{
    expandShrink(true);
}

void ExpandShrinkDialog::expandShrink(bool shrink)
{
    int factor = ui->spinBoxFactor->value();

    if (factor < 2) {
        return;
    }

    if (shrink) {
        factor = -factor;
    }

    switch (ui->comboBoxArea->currentIndex()) {
    case AreaSong:
        song->expandShrink(factor);
        break;
    case AreaBlock: {
        Block *block = song->block(this->block);
        block->expandShrink(factor, 0, 0, block->tracks() - 1, block->length() - 1);
        break;
    }
    case AreaTrack: {
        Block *block = song->block(this->block);
        block->expandShrink(factor, track, 0, track, block->length() - 1);
        break;
    }
    case AreaSelection:
        if (selectionStartTrack >= 0 && selectionStartLine >= 0 && selectionEndTrack >= 0 && selectionEndLine >= 0) {
            Block *block = song->block(this->block);
            block->expandShrink(factor, selectionStartTrack, selectionStartLine, selectionEndTrack, selectionEndLine);
        }
        break;
    default:
        break;
    }
}
