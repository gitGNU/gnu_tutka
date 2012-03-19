/*
 * playingsequencelistdialog.cpp
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
#include "playingsequencelisttablemodel.h"
#include "playingsequencelistdialog.h"
#include "ui_playingsequencelistdialog.h"

PlayingSequenceListDialog::PlayingSequenceListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayingSequenceListDialog),
    song(NULL),
    playingSequenceListTableModel(new PlayingSequenceListTableModel(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(playingSequenceListTableModel);

    connect(ui->pushButtonInsertNew, SIGNAL(clicked()), this, SLOT(insertPlayingSequence()));
    connect(ui->pushButtonAppendNew, SIGNAL(clicked()), this, SLOT(appendPlayingSequence()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deletePlayingSequence()));
}

PlayingSequenceListDialog::~PlayingSequenceListDialog()
{
    delete ui;
}

void PlayingSequenceListDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void PlayingSequenceListDialog::setSong(Song *song)
{
    this->song = song;
    playingSequenceListTableModel->setSong(song);
}

void PlayingSequenceListDialog::setPlayingSequence(unsigned int playingSequence)
{
    ui->tableView->selectRow(playingSequence);
}

void PlayingSequenceListDialog::insertPlayingSequence()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertPlayseq(indexes.first().row());
    }
}

void PlayingSequenceListDialog::appendPlayingSequence()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertPlayseq(song->playseqs());
    }
}

void PlayingSequenceListDialog::deletePlayingSequence()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deletePlayseq(indexes.first().row());
    }
}
