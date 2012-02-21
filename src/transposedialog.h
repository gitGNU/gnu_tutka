#ifndef TRANSPOSEDIALOG_H
#define TRANSPOSEDIALOG_H

#include <QDialog>

namespace Ui {
    class TransposeDialog;
}

class Song;

class TransposeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransposeDialog(QWidget *parent = 0);
    ~TransposeDialog();

public slots:
    void showSong();
    void showBlock();
    void showTrack();
    void showSelection();
    void setSong(Song *song);
    void setBlock(unsigned int block);
    void setTrack(int track);
    void setInstrument(int instrument);
    void setSelection(int startTrack, int startLine, int endTrack, int endLine);

private slots:
    void transpose();

private:
    enum Area {
      SONG,
      BLOCK,
      TRACK,
      SELECTION
    };

    Ui::TransposeDialog *ui;
    Song *song;
    unsigned int block;
    unsigned int track;
    unsigned int instrument;
    int selectionStartTrack;
    int selectionStartLine;
    int selectionEndTrack;
    int selectionEndLine;
};

#endif // TRANSPOSEDIALOG_H
