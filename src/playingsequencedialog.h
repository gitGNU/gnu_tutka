#ifndef PLAYINGSEQUENCEDIALOG_H
#define PLAYINGSEQUENCEDIALOG_H

#include <QDialog>

namespace Ui {
    class PlayingSequenceDialog;
}

class PlayingSequenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlayingSequenceDialog(QWidget *parent = 0);
    ~PlayingSequenceDialog();

private:
    Ui::PlayingSequenceDialog *ui;
};

#endif // PLAYINGSEQUENCEDIALOG_H
