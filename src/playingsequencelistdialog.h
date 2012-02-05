#ifndef PLAYINGSEQUENCELISTDIALOG_H
#define PLAYINGSEQUENCELISTDIALOG_H

#include <QDialog>

namespace Ui {
    class PlayingSequenceListDialog;
}

class PlayingSequenceListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayingSequenceListDialog(QWidget *parent = 0);
    ~PlayingSequenceListDialog();

private:
    Ui::PlayingSequenceListDialog *ui;
};

#endif // PLAYINGSEQUENCELISTDIALOG_H
