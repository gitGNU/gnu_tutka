#include "blocklistdialog.h"
#include "ui_blocklistdialog.h"

BlockListDialog::BlockListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockListDialog)
{
    ui->setupUi(this);
}

BlockListDialog::~BlockListDialog()
{
    delete ui;
}
