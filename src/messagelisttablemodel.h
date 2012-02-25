#ifndef MESSAGELISTTABLEMODEL_H
#define MESSAGELISTTABLEMODEL_H

#include <QAbstractTableModel>

class Song;

class MessageListTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MessageListTableModel(QObject *parent = 0);

    void setSong(Song *song);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    void refresh();

private:
    Song *song;
};

#endif // MESSAGELISTTABLEMODEL_H
