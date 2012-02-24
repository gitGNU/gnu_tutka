#include "song.h"
#include "playseq.h"
#include "playingsequencelisttablemodel.h"

PlayingSequenceListTableModel::PlayingSequenceListTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    song(NULL)
{
}

void PlayingSequenceListTableModel::setSong(Song *song)
{
    beginResetModel();
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(playseqsChanged(uint)), this, SLOT(refresh()));
    }
    this->song = song;
    connect(this->song, SIGNAL(playseqsChanged(uint)), this, SLOT(refresh()));
    endResetModel();
}

int PlayingSequenceListTableModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid() || song == NULL) ? 0 : song->playseqs();
}

int PlayingSequenceListTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant PlayingSequenceListTableModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= song->playseqs() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
        return QVariant();
    } else {
        switch (index.column()) {
        case 0:
            return song->playseq(index.row())->name();
        default:
            return QVariant();
        }
    }
}

QVariant PlayingSequenceListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch(section) {
            case 0:
                return QVariant("Name");
            default:
                break;
            }
        } else {
            return QVariant(section + 1);
        }
    }
    return QVariant();
}

bool PlayingSequenceListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.row() >= 0 && index.row() < song->playseqs()) {
        bool success = false;

        switch(index.column()) {
        case 0:
            song->playseq(index.row())->setName(value.toString());
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

Qt::ItemFlags PlayingSequenceListTableModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)

    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
}

void PlayingSequenceListTableModel::refresh()
{
    beginResetModel();
    endResetModel();
}
