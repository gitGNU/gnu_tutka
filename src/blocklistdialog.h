#ifndef BLOCKLISTDIALOG_H
#define BLOCKLISTDIALOG_H

#include <QDialog>

namespace Ui {
    class BlockListDialog;
}

class Song;
class BlockListTableModel;
class SpinBoxDelegate;

class BlockListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlockListDialog(QWidget *parent = 0);
    ~BlockListDialog();

public slots:
    void setSong(Song *song);
    void setBlock(unsigned int block);

private slots:
    void insertBlock();
    void appendBlock();
    void deleteBlock();

private:
    Ui::BlockListDialog *ui;
    Song *song;
    BlockListTableModel *blockListTableModel;
    SpinBoxDelegate *spinBoxDelegate;
};

#endif // BLOCKLISTDIALOG_H
