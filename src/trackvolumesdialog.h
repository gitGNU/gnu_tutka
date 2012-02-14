#ifndef TRACKVOLUMESDIALOG_H
#define TRACKVOLUMESDIALOG_H

#include <QDialog>

namespace Ui {
    class TrackVolumesDialog;
}

class Song;

class TrackVolumesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrackVolumesDialog(QWidget *parent = 0);
    ~TrackVolumesDialog();

public slots:
    void setSong(Song *song);

private slots:
    void setTracks(unsigned int tracks);

private:
    Ui::TrackVolumesDialog *ui;
    Song *song;
};

#endif // TRACKVOLUMESDIALOG_H
