#include "song.h"
#include "spinboxdelegate.h"
#include "blocklisttablemodel.h"
#include "blocklistdialog.h"
#include "ui_blocklistdialog.h"

BlockListDialog::BlockListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockListDialog),
    song(NULL),
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

    connect(ui->pushButtonInsertNew, SIGNAL(clicked()), this, SLOT(insertBlock()));
    connect(ui->pushButtonAppendNew, SIGNAL(clicked()), this, SLOT(appendBlock()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteBlock()));
}

BlockListDialog::~BlockListDialog()
{
    delete ui;
}

void BlockListDialog::setSong(Song *song)
{
    this->song = song;
    blockListTableModel->setSong(song);
}

void BlockListDialog::setBlock(unsigned int block)
{
    ui->tableView->selectRow(block);
}

void BlockListDialog::insertBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertBlock(indexes.first().row(), indexes.first().row());
    }
}

void BlockListDialog::appendBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertBlock(song->blocks(), indexes.first().row());
    }
}

void BlockListDialog::deleteBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deleteBlock(indexes.first().row());
    }
}
