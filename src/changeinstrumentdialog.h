#ifndef CHANGEINSTRUMENTDIALOG_H
#define CHANGEINSTRUMENTDIALOG_H

#include <QDialog>

namespace Ui {
    class ChangeInstrumentDialog;
}

class Song;

class ChangeInstrumentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeInstrumentDialog(QWidget *parent = 0);
    ~ChangeInstrumentDialog();

public slots:
    void showSong();
    void showBlock();
    void showTrack();
    void showSelection();
    void setSong(Song *song);
    void setBlock(unsigned int block);
    void setTrack(int track);

private slots:
    void swap();
    void change(bool swap = false);

private:
    enum Area {
      SONG,
      BLOCK,
      TRACK,
      SELECTION
    };

    Ui::ChangeInstrumentDialog *ui;
    Song *song;
    unsigned int block;
    unsigned int track;
};

#endif // CHANGEINSTRUMENTDIALOG_H
