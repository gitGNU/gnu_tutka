/*
 * playingsequencedialog.cpp
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
    spinBoxDelegate(new SpinBoxDelegate(1, 1, this)),
    playseqNumber(0),
    position(-1)
{
    ui->setupUi(this);

    ui->tableView->setModel(playingSequenceTableModel);
    ui->tableView->setItemDelegateForColumn(0, spinBoxDelegate);

    connect(ui->pushButtonInsert, SIGNAL(clicked()), this, SLOT(insertBlock()));
    connect(ui->pushButtonAppend, SIGNAL(clicked()), this, SLOT(appendBlock()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteBlock()));
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(setPosition(QItemSelection, QItemSelection)));
    connect(playingSequenceTableModel, SIGNAL(modelReset()), this, SLOT(setSelection()));
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
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(blocksChanged(int)), spinBoxDelegate, SLOT(setMaximum(int)));
    }

    this->song = song;
    connect(this->song, SIGNAL(blocksChanged(int)), spinBoxDelegate, SLOT(setMaximum(int)));
    playingSequenceTableModel->setSong(song);
}

void PlayingSequenceDialog::setPlayseq(unsigned int playseq)
{
    if (this->playseq != NULL) {
        disconnect(this->playseq, SIGNAL(nameChanged(QString)), this, SLOT(setWindowTitle()));
        disconnect(this->playseq, SIGNAL(playseqChanged()), this, SLOT(setDeleteButtonVisibility()));
    }

    this->playseqNumber = playseq;
    this->playseq = song->playseq(playseq);
    playingSequenceTableModel->setPlayseq(this->playseq);

    connect(this->playseq, SIGNAL(nameChanged(QString)), this, SLOT(setWindowTitle()));
    connect(this->playseq, SIGNAL(playseqChanged()), this, SLOT(setDeleteButtonVisibility()));
    setWindowTitle();
    setDeleteButtonVisibility();
}

void PlayingSequenceDialog::setPosition(unsigned int position)
{
    ui->tableView->selectRow(position);
    this->position = position;
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

void PlayingSequenceDialog::setSelection()
{
    if (position >= 0) {
        ui->tableView->selectRow(position);
    }
}

void PlayingSequenceDialog::setPosition(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    QModelIndexList indexes = selected.indexes();
    if (!indexes.isEmpty()) {
        int position = indexes.first().row();
        if (position != this->position) {
            this->position = position;
            emit positionSelected(this->position);
        }
    }
}

void PlayingSequenceDialog::setWindowTitle()
{
    QString name = playseq->name();
    if (name.isEmpty()) {
        QDialog::setWindowTitle(tr("Playing Sequence %1/%2").arg(playseqNumber + 1).arg(song->playseqs()));
    } else {
        QDialog::setWindowTitle(tr("Playing Sequence %1/%2: %3").arg(playseqNumber + 1).arg(song->playseqs()).arg(name));
    }
}

void PlayingSequenceDialog::setDeleteButtonVisibility()
{
    ui->pushButtonDelete->setEnabled(playseq != NULL && playseq->length() > 1);
}
