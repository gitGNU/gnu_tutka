#include <QSlider>
#include "song.h"
#include "trackvolumesdialog.h"
#include "ui_trackvolumesdialog.h"

TrackVolumesDialog::TrackVolumesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrackVolumesDialog),
    song(NULL)
{
    ui->setupUi(this);
}

TrackVolumesDialog::~TrackVolumesDialog()
{
    delete ui;
}

void TrackVolumesDialog::setSong(Song *song)
{
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(maxTracksChanged(uint)), this, SLOT(setTracks(uint)));
    }

    this->song = song;
    setTracks(this->song->maxTracks());

    connect(this->song, SIGNAL(maxTracksChanged(uint)), this, SLOT(setTracks(uint)));
}

void TrackVolumesDialog::setTracks(unsigned int tracks)
{
    while(ui->scrollAreaWidgetContents->layout()->count() > 0) {
        QLayoutItem *item = ui->scrollAreaWidgetContents->layout()->takeAt(0);
        delete item->widget();
        delete item;
    }

    for (int i = 0; i < tracks; i++) {
        ui->scrollAreaWidgetContents->layout()->addWidget(new QSlider);
    }
}
