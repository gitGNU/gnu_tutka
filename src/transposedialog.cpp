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

void TransposeDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void TransposeDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void TransposeDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void TransposeDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}
