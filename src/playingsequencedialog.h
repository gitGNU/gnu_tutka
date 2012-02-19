#ifndef PLAYINGSEQUENCEDIALOG_H
#define PLAYINGSEQUENCEDIALOG_H

#include <QDialog>

namespace Ui {
    class PlayingSequenceDialog;
}

class Song;
class Playseq;
class PlayingSequenceTableModel;
class SpinBoxDelegate;

class PlayingSequenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayingSequenceDialog(QWidget *parent = 0);
    ~PlayingSequenceDialog();

public slots:
    void setSong(Song *song);
    void setPlayseq(unsigned int playseq);
    void setPosition(unsigned int position);

private slots:
    void insertBlock();
    void appendBlock();
    void deleteBlock();

private:
    Ui::PlayingSequenceDialog *ui;
    Song *song;
    Playseq *playseq;
    PlayingSequenceTableModel *playingSequenceTableModel;
    SpinBoxDelegate *spinBoxDelegate;
};

#endif // PLAYINGSEQUENCEDIALOG_H
