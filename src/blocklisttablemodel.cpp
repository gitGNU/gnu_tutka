/*
 * blocklisttablemodel.cpp
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
#include "blocklisttablemodel.h"

BlockListTableModel::BlockListTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    song(NULL)
{
}

void BlockListTableModel::setSong(Song *song)
{
    beginResetModel();
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(blocksChanged(int)), this, SLOT(refresh()));
    }
    this->song = song;
    connect(this->song, SIGNAL(blocksChanged(int)), this, SLOT(refresh()));
    endResetModel();
}

int BlockListTableModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || song == NULL) ? 0 : song->blocks();
}

int BlockListTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 4;
}

QVariant BlockListTableModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= song->blocks() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
        return QVariant();
    } else {
        switch (index.column()) {
        case 0:
            return song->block(index.row())->name();
        case 1:
            return song->block(index.row())->tracks();
        case 2:
            return song->block(index.row())->length();
        case 3:
            return song->block(index.row())->commandPages();
        default:
            return QVariant();
        }
    }
}

QVariant BlockListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Tracks");
            case 2:
                return tr("Length");
            case 3:
                return tr("Command pages");
            default:
                break;
            }
        } else {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

bool BlockListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.row() >= 0 && index.row() < song->blocks()) {
        bool success = false;

        switch(index.column()) {
        case 0:
            song->block(index.row())->setName(value.toString());
            success = true;
            break;
        case 1:
            song->block(index.row())->setTracks(value.toUInt(&success));
            break;
        case 2:
            song->block(index.row())->setLength(value.toUInt(&success));
            break;
        case 3:
            song->block(index.row())->setCommandPages(value.toUInt(&success));
            break;
        default:
            break;
        }

        return success;
    } else {
        return false;
    }
}

Qt::ItemFlags BlockListTableModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)

    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

void BlockListTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}
