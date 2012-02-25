#include "song.h"
#include "spinboxdelegate.h"
#include "messagelisttablemodel.h"
#include "messagelistdialog.h"
#include "ui_messagelistdialog.h"

MessageListDialog::MessageListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageListDialog),
    song(NULL),
    messageListTableModel(new MessageListTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(messageListTableModel);
    ui->tableView->setColumnWidth(0, 200);
    ui->tableView->setItemDelegateForColumn(1, spinBoxDelegate);

    connect(ui->pushButtonInsertNew, SIGNAL(clicked()), this, SLOT(insertMessage()));
    connect(ui->pushButtonAppendNew, SIGNAL(clicked()), this, SLOT(appendMessage()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteMessage()));
}

MessageListDialog::~MessageListDialog()
{
    delete ui;
}

void MessageListDialog::setSong(Song *song)
{
    this->song = song;
    messageListTableModel->setSong(song);
}

void MessageListDialog::insertMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertMessage(indexes.first().row());
    }
}

void MessageListDialog::appendMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertMessage(song->messages());
    }
}

void MessageListDialog::deleteMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deleteMessage(indexes.first().row());
    }
}
