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

private:
    Ui::ExpandShrinkDialog *ui;
};

#endif // EXPANDSHRINKDIALOG_H
