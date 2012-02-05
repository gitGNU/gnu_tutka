#include "sectionlistdialog.h"
#include "ui_sectionlistdialog.h"

SectionListDialog::SectionListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SectionListDialog)
{
    ui->setupUi(this);
}

SectionListDialog::~SectionListDialog()
{
    delete ui;
}
