#ifndef INSTRUMENTPROPERTIESDIALOG_H
#define INSTRUMENTPROPERTIESDIALOG_H

#include <QDialog>

class Song;

namespace Ui {
    class InstrumentPropertiesDialog;
}

class InstrumentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InstrumentPropertiesDialog(QWidget *parent = 0);
    ~InstrumentPropertiesDialog();

public slots:
    void setSong(Song *song);
    void setInstrument(int number);

private slots:
    void setName(const QString &name);
    void setMidiChannel(int midiChannel);
    void setVolume(int volume);
    void setTranspose(int transpose);
    void setHold(int hold);

private:
    Ui::InstrumentPropertiesDialog *ui;
    Song *song;
    int instrument;
};

#endif // INSTRUMENTPROPERTIESDIALOG_H
