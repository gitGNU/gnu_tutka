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

void ExpandShrinkDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void ExpandShrinkDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void ExpandShrinkDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void ExpandShrinkDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}
