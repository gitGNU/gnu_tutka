#ifndef INPUTMIDIINTERFACESTABLEMODEL_H
#define INPUTMIDIINTERFACESTABLEMODEL_H

#include <QAbstractTableModel>

class MIDI;

class InputMidiInterfacesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit InputMidiInterfacesTableModel(MIDI *midi, QObject *parent = NULL);
    
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

#endif // INPUTMIDIINTERFACESTABLEMODEL_H
