#include "changeinstrumentdialog.h"
#include "ui_changeinstrumentdialog.h"

ChangeInstrumentDialog::ChangeInstrumentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeInstrumentDialog)
{
    ui->setupUi(this);
}

ChangeInstrumentDialog::~ChangeInstrumentDialog()
{
    delete ui;
}

void ChangeInstrumentDialog::showSong()
{
    ui->comboBoxArea->setCurrentIndex(0);
    show();
}

void ChangeInstrumentDialog::showBlock()
{
    ui->comboBoxArea->setCurrentIndex(1);
    show();
}

void ChangeInstrumentDialog::showTrack()
{
    ui->comboBoxArea->setCurrentIndex(2);
    show();
}

void ChangeInstrumentDialog::showSelection()
{
    ui->comboBoxArea->setCurrentIndex(3);
    show();
}
