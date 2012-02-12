#ifndef BLOCKLISTDIALOG_H
#define BLOCKLISTDIALOG_H

#include <QDialog>

namespace Ui {
    class BlockListDialog;
}

class BlockListTableModel;
class SpinBoxDelegate;
class Song;

class BlockListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlockListDialog(QWidget *parent = 0);
    ~BlockListDialog();

public slots:
    void setSong(Song *song);
    void setBlock(unsigned int block);

private:
    Ui::BlockListDialog *ui;
    BlockListTableModel *blockListTableModel;
    SpinBoxDelegate *spinBoxDelegate;
};

#endif // BLOCKLISTDIALOG_H
