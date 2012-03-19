/*
 * playingsequencedialog.cpp
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
#include "playseq.h"
#include "spinboxdelegate.h"
#include "playingsequencetablemodel.h"
#include "playingsequencedialog.h"
#include "ui_playingsequencedialog.h"

PlayingSequenceDialog::PlayingSequenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlayingSequenceDialog),
    song(NULL),
    playseq(NULL),
    playingSequenceTableModel(new PlayingSequenceTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(playingSequenceTableModel);
    ui->tableView->setItemDelegateForColumn(1, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(2, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(3, spinBoxDelegate);

    connect(ui->pushButtonInsert, SIGNAL(clicked()), this, SLOT(insertBlock()));
    connect(ui->pushButtonAppend, SIGNAL(clicked()), this, SLOT(appendBlock()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteBlock()));
}

PlayingSequenceDialog::~PlayingSequenceDialog()
{
    delete ui;
}

void PlayingSequenceDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void PlayingSequenceDialog::setSong(Song *song)
{
    this->song = song;
    playingSequenceTableModel->setSong(song);
}

void PlayingSequenceDialog::setPlayseq(unsigned int playseq)
{
    this->playseq = song->playseq(playseq);
    playingSequenceTableModel->setPlayseq(this->playseq);
}

void PlayingSequenceDialog::setPosition(unsigned int position)
{
    ui->tableView->selectRow(position);
}

void PlayingSequenceDialog::insertBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        playseq->insert(indexes.first().row());
    }
}

void PlayingSequenceDialog::appendBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        playseq->insert(playseq->length());
    }
}

void PlayingSequenceDialog::deleteBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        playseq->remove(indexes.first().row());
    }
}
