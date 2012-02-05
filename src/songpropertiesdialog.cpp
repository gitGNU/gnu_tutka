#include "songpropertiesdialog.h"
#include "ui_songpropertiesdialog.h"

SongPropertiesDialog::SongPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SongPropertiesDialog)
{
    ui->setupUi(this);
}

SongPropertiesDialog::~SongPropertiesDialog()
{
    delete ui;
}
