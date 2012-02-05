#include "messagelistdialog.h"
#include "ui_messagelistdialog.h"

MessageListDialog::MessageListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageListDialog)
{
    ui->setupUi(this);
}

MessageListDialog::~MessageListDialog()
{
    delete ui;
}
