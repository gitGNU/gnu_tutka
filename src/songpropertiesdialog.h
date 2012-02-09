#ifndef SONGPROPERTIESDIALOG_H
#define SONGPROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
    class SongPropertiesDialog;
}

class Song;

class SongPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SongPropertiesDialog(QWidget *parent = 0);
    ~SongPropertiesDialog();

public slots:
    void setSong(Song *song);

private:
    Ui::SongPropertiesDialog *ui;
    Song *song;
};

#endif // SONGPROPERTIESDIALOG_H
