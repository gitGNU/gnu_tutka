#include "song.h"
#include "sectiontablemodel.h"

SectionTableModel::SectionTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    song(NULL)
{
}

void SectionTableModel::setSong(Song *song)
{
    beginResetModel();
    disconnect(this->song, SIGNAL(sectionsChanged(uint)), this, SLOT(refresh()));
    this->song = song;
    connect(this->song, SIGNAL(sectionsChanged(uint)), this, SLOT(refresh()));
    endResetModel();
}

int SectionTableModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || song == NULL) ? 0 : song->sections();
}

int SectionTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant SectionTableModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= song->sections() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
        return QVariant();
    } else {
        switch (index.column()) {
        case 0:
            return song->section(index.row());
        case 1:
            return song->playseq(song->section(index.row()))->name();
        default:
            return QVariant();
        }
    }
}

QVariant SectionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return QVariant("Playing Sequence Number");
            case 1:
                return QVariant("Playing Sequence Name");
            default:
                break;
            }
        } else {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

bool SectionTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.row() >= 0 && index.row() < song->sections()) {
        bool success = false;

        switch(index.column()) {
        case 0:
            song->setSection(index.row(), value.toInt());
            success = true;
            break;
        default:
            break;
        }

        return success;
    } else {
        return false;
    }
}

Qt::ItemFlags SectionTableModel::flags(const QModelIndex &index) const
{
    return index.column() < 1 ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable) : (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void SectionTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}
