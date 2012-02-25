/*
 * playingsequencetablemodel.cpp
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
#include "playingsequencetablemodel.h"

PlayingSequenceTableModel::PlayingSequenceTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    song(NULL),
    playseq(NULL)
{
}

void PlayingSequenceTableModel::setSong(Song *song)
{
    beginResetModel();
    this->song = song;
    endResetModel();
}

void PlayingSequenceTableModel::setPlayseq(Playseq *playseq)
{
    beginResetModel();
    if (this->playseq != NULL) {
        disconnect(this->playseq, SIGNAL(playseqChanged()), this, SLOT(refresh()));
    }
    this->playseq = playseq;
    connect(this->playseq, SIGNAL(playseqChanged()), this, SLOT(refresh()));
    endResetModel();
}

int PlayingSequenceTableModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || playseq == NULL) ? 0 : playseq->length();
}

int PlayingSequenceTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant PlayingSequenceTableModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= playseq->length() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
        return QVariant();
    } else {
        switch (index.column()) {
        case 0:
            return playseq->at(index.row());
        case 1:
            return song->block(playseq->at(index.row()))->name();
        default:
            return QVariant();
        }
    }
}

QVariant PlayingSequenceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return QVariant("Block Number");
            case 1:
                return QVariant("Block Name");
            default:
                break;
            }
        } else {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

bool PlayingSequenceTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.row() >= 0 && index.row() < playseq->length()) {
        bool success = false;

        switch(index.column()) {
        case 0:
            playseq->set(index.row(), value.toInt(&success));
            break;
        default:
            break;
        }

        return success;
    } else {
        return false;
    }
}

Qt::ItemFlags PlayingSequenceTableModel::flags(const QModelIndex &index) const
{
    return index.column() < 1 ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable) : (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void PlayingSequenceTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}
