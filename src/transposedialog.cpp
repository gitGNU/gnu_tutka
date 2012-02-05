#include "transposedialog.h"
#include "ui_transposedialog.h"

TransposeDialog::TransposeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransposeDialog)
{
    ui->setupUi(this);
}

TransposeDialog::~TransposeDialog()
{
    delete ui;
}
