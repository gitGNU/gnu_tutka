#ifndef TRANSPOSEDIALOG_H
#define TRANSPOSEDIALOG_H

#include <QDialog>

namespace Ui {
    class TransposeDialog;
}

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

private:
    Ui::TransposeDialog *ui;
};

#endif // TRANSPOSEDIALOG_H
