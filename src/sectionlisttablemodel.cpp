#include "song.h"
#include "sectionlisttablemodel.h"

SectionListTableModel::SectionListTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    song(NULL)
{
}

void SectionListTableModel::setSong(Song *song)
{
    beginResetModel();
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(sectionsChanged(uint)), this, SLOT(refresh()));
    }
    this->song = song;
    connect(this->song, SIGNAL(sectionsChanged(uint)), this, SLOT(refresh()));
    endResetModel();
}

int SectionListTableModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || song == NULL) ? 0 : song->sections();
}

int SectionListTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant SectionListTableModel::data(const QModelIndex &index, int role) const
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

QVariant SectionListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool SectionListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

Qt::ItemFlags SectionListTableModel::flags(const QModelIndex &index) const
{
    return index.column() < 1 ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable) : (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

void SectionListTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}
