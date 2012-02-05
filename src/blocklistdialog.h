#ifndef BLOCKLISTDIALOG_H
#define BLOCKLISTDIALOG_H

#include <QDialog>

namespace Ui {
    class BlockListDialog;
}

class BlockListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlockListDialog(QWidget *parent = 0);
    ~BlockListDialog();

private:
    Ui::BlockListDialog *ui;
};

#endif // BLOCKLISTDIALOG_H
