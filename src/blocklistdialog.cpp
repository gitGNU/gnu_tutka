#include "spinboxdelegate.h"
#include "blocklisttablemodel.h"
#include "blocklistdialog.h"
#include "ui_blocklistdialog.h"

BlockListDialog::BlockListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockListDialog),
    blockListTableModel(new BlockListTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(blockListTableModel);
    ui->tableView->setColumnWidth(0, 200);
    ui->tableView->setColumnWidth(3, 100);
    ui->tableView->setItemDelegateForColumn(1, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(2, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(3, spinBoxDelegate);
}

BlockListDialog::~BlockListDialog()
{
    delete ui;
}

void BlockListDialog::setSong(Song *song)
{
    blockListTableModel->setSong(song);
}

void BlockListDialog::setBlock(unsigned int block)
{
    Q_UNUSED(block)
}
