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
