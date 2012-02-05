#include "instrumentpropertiesdialog.h"
#include "ui_instrumentpropertiesdialog.h"

InstrumentPropertiesDialog::InstrumentPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InstrumentPropertiesDialog)
{
    ui->setupUi(this);
}

InstrumentPropertiesDialog::~InstrumentPropertiesDialog()
{
    delete ui;
}
