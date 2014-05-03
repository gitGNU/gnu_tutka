/*
 * inputmidiinterfacestablemodel.cpp
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

#include "midi.h"
#include "midiinterface.h"
#include "inputmidiinterfacestablemodel.h"

InputMidiInterfacesTableModel::InputMidiInterfacesTableModel(MIDI *midi, QObject *parent) :
    QAbstractTableModel(parent),
    midi(midi)
{
    connect(midi, SIGNAL(inputsChanged()), this, SLOT(refresh()));
}

int InputMidiInterfacesTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : midi->inputs();
}

int InputMidiInterfacesTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant InputMidiInterfacesTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return midi->input(index.row())->name();
        default:
            break;
        }
    } else if (role == Qt::CheckStateRole && index.column() == 1) {
        return midi->input(index.row())->isEnabled() ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
    }
    return QVariant();
}

QVariant InputMidiInterfacesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Enabled");
            default:
                break;
            }
        }
    }
    return QVariant();
}

bool InputMidiInterfacesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 1) {
        midi->input(index.row())->setEnabled(value == QVariant(Qt::Checked));
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags InputMidiInterfacesTableModel::flags(const QModelIndex &index) const
{
    return index.column() == 1 ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable) : (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void InputMidiInterfacesTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}
