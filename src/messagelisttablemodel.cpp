/*
 * messagelisttablemodel.cpp
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
#include "message.h"
#include "messagelisttablemodel.h"

MessageListTableModel::MessageListTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    song(NULL)
{
}

void MessageListTableModel::setSong(Song *song)
{
    beginResetModel();
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(messagesChanged(uint)), this, SLOT(refresh()));
    }
    this->song = song;
    connect(this->song, SIGNAL(messagesChanged(uint)), this, SLOT(refresh()));
    endResetModel();
}

int MessageListTableModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || song == NULL) ? 0 : song->messages();
}

int MessageListTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 3;
}

QVariant MessageListTableModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() < song->messages()) {
        if (role == Qt::DisplayRole || role == Qt::EditRole) {
            switch (index.column()) {
            case 0:
                return song->message(index.row())->name();
            case 1:
                return song->message(index.row())->length();
            default:
                break;
            }
        } else if (role == Qt::CheckStateRole && index.column() == 2) {
            return song->message(index.row())->isAutoSend() ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
        }
    }
    return QVariant();
}

QVariant MessageListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Length");
            case 2:
                return tr("Automatically Send Message After Loading Song");
            default:
                break;
            }
        } else {
            return QVariant(section);
        }
    }
    return QVariant();
}

bool MessageListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool success = false;

    if (index.row() >= 0 && index.row() < song->messages()) {
        if (role == Qt::EditRole) {
            switch(index.column()) {
            case 0:
                song->message(index.row())->setName(value.toString());
                success = true;
                break;
            case 1:
                song->message(index.row())->setLength(value.toUInt(&success));
                break;
            default:
                break;
            }
        } else if (role == Qt::CheckStateRole && index.column() == 2) {
            song->message(index.row())->setAutoSend(value == QVariant(Qt::Checked));
            success = true;
        }
    }

    return success;
}

Qt::ItemFlags MessageListTableModel::flags(const QModelIndex &index) const
{
    return index.column() == 2 ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable) : (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
}

void MessageListTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}
