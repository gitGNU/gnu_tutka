#ifndef PLAYINGSEQUENCELISTDIALOG_H
#define PLAYINGSEQUENCELISTDIALOG_H

#include <QDialog>

namespace Ui {
    class PlayingSequenceListDialog;
}

class Song;
class PlayingSequenceListTableModel;

class PlayingSequenceListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayingSequenceListDialog(QWidget *parent = 0);
    ~PlayingSequenceListDialog();

public slots:
    void setSong(Song *song);
    void setPlayingSequence(unsigned int playingSequence);

private slots:
    void insertPlayingSequence();
    void appendPlayingSequence();
    void deletePlayingSequence();

private:
    Ui::PlayingSequenceListDialog *ui;
    Song *song;
    PlayingSequenceListTableModel *playingSequenceListTableModel;
};

#endif // PLAYINGSEQUENCELISTDIALOG_H
