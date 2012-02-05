#include "trackvolumesdialog.h"
#include "ui_trackvolumesdialog.h"

TrackVolumesDialog::TrackVolumesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrackVolumesDialog)
{
    ui->setupUi(this);
}

TrackVolumesDialog::~TrackVolumesDialog()
{
    delete ui;
}
