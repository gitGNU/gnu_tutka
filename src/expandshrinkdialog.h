#ifndef EXPANDSHRINKDIALOG_H
#define EXPANDSHRINKDIALOG_H

#include <QDialog>

namespace Ui {
    class ExpandShrinkDialog;
}

class Song;

class ExpandShrinkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExpandShrinkDialog(QWidget *parent = 0);
    ~ExpandShrinkDialog();

public slots:
    void showSong();
    void showBlock();
    void showTrack();
    void showSelection();
    void setSong(Song *song);
    void setBlock(unsigned int block);
    void setTrack(int track);
    void setSelection(int startTrack, int startLine, int endTrack, int endLine);

private slots:
    void expand();
    void shrink();

private:
    enum Area {
      SONG,
      BLOCK,
      TRACK,
      SELECTION
    };

    Ui::ExpandShrinkDialog *ui;
    Song *song;
    unsigned int block;
    unsigned int track;
    int selectionStartTrack;
    int selectionStartLine;
    int selectionEndTrack;
    int selectionEndLine;

    void expandShrink(bool shrink);
};

#endif // EXPANDSHRINKDIALOG_H
