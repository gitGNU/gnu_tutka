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

private:
    Ui::InstrumentPropertiesDialog *ui;
    Song *song;
    int instrument;
};

#endif // INSTRUMENTPROPERTIESDIALOG_H
