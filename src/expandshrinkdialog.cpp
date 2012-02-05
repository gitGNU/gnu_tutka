#include "expandshrinkdialog.h"
#include "ui_expandshrinkdialog.h"

ExpandShrinkDialog::ExpandShrinkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExpandShrinkDialog)
{
    ui->setupUi(this);
}

ExpandShrinkDialog::~ExpandShrinkDialog()
{
    delete ui;
}
