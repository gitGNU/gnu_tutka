#ifndef OUTPUTMIDIINTERFACESTABLEMODEL_H
#define OUTPUTMIDIINTERFACESTABLEMODEL_H

#include <QAbstractTableModel>

class MIDI;

class OutputMidiInterfacesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit OutputMidiInterfacesTableModel(MIDI *midi, QObject *parent = NULL);
    
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

private slots:
    void refresh();

private:
    MIDI *midi;
};

#endif // OUTPUTMIDIINTERFACESTABLEMODEL_H
