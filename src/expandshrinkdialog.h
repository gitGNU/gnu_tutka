#ifndef EXPANDSHRINKDIALOG_H
#define EXPANDSHRINKDIALOG_H

#include <QDialog>

namespace Ui {
    class ExpandShrinkDialog;
}

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

private:
    Ui::ExpandShrinkDialog *ui;
};

#endif // EXPANDSHRINKDIALOG_H
