#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include "song.h"
#include "trackvolumewidget.h"

TrackVolumeWidget::TrackVolumeWidget(unsigned int number, Track *track, QWidget *parent) :
    QWidget(parent),
    trackNumber(number),
    track(track),
    volumeSlider(new QSlider),
    muteCheckBox(new QCheckBox("Mute")),
    soloCheckBox(new QCheckBox("Solo")),
    nameLineEdit(new QLineEdit)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(QString("Track %1").arg(number)));
    layout->addWidget(volumeSlider);
    layout->addWidget(muteCheckBox);
    layout->addWidget(soloCheckBox);
    layout->addWidget(nameLineEdit);

    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(127);
    volumeSlider->setValue(track->volume());
    muteCheckBox->setChecked(track->isMuted());
    soloCheckBox->setChecked(track->isSolo());
    nameLineEdit->setText(track->name());

    connect(volumeSlider, SIGNAL(valueChanged(int)), track, SLOT(setVolume(int)));
    connect(muteCheckBox, SIGNAL(toggled(bool)), track, SLOT(setMute(bool)));
    connect(soloCheckBox, SIGNAL(toggled(bool)), track, SLOT(setSolo(bool)));
    connect(nameLineEdit, SIGNAL(textChanged(QString)), track, SLOT(setName(QString)));
}
