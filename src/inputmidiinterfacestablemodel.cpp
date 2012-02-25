#include "midi.h"
#include "midiinterface.h"
#include "inputmidiinterfacestablemodel.h"

InputMidiInterfacesTableModel::InputMidiInterfacesTableModel(MIDI *midi, QObject *parent) :
    QAbstractTableModel(parent),
    midi(midi)
{
}

int InputMidiInterfacesTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (midi->inputs() - 1);
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
            return midi->input(index.row() + 1)->name();
        default:
            break;
        }
    } else if (role == Qt::CheckStateRole && index.column() == 1) {
        return midi->input(index.row() + 1)->isEnabled() ? QVariant(Qt::Checked) : QVariant(Qt::Unchecked);
    }
    return QVariant();
}

QVariant InputMidiInterfacesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return QVariant("Name");
            case 1:
                return QVariant("Enabled");
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
        midi->input(index.row() + 1)->setEnabled(value == QVariant(Qt::Checked));
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
