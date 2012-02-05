#include "song.h"
#include "instrumentpropertiesdialog.h"
#include "ui_instrumentpropertiesdialog.h"

InstrumentPropertiesDialog::InstrumentPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InstrumentPropertiesDialog),
    song(NULL),
    instrument(0)
{
    ui->setupUi(this);
}

InstrumentPropertiesDialog::~InstrumentPropertiesDialog()
{
    delete ui;
}

void InstrumentPropertiesDialog::setSong(Song *song)
{
    this->song = song;
}

void InstrumentPropertiesDialog::setInstrument(int number)
{
    if (number > 0) {
        this->instrument = number - 1;
        song->checkInstrument(this->instrument, 0);

        Instrument *instrument = song->instrument(this->instrument);

        ui->lineEditName->setText(instrument->name());
        ui->horizontalSliderMidiChannel->setValue(instrument->midiChannel() + 1);
        ui->horizontalSliderVolume->setValue(instrument->defaultVelocity());
        ui->horizontalSliderTranspose->setValue(instrument->transpose());
        ui->horizontalSliderHold->setValue(instrument->hold());

        connect(ui->lineEditName, SIGNAL(textEdited(QString)), this, SLOT(setName(QString)));
        connect(ui->horizontalSliderMidiChannel, SIGNAL(valueChanged(int)), this, SLOT(setMidiChannel(int)));
        connect(ui->horizontalSliderVolume, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
        connect(ui->horizontalSliderTranspose, SIGNAL(valueChanged(int)), this, SLOT(setTranspose(int)));
        connect(ui->horizontalSliderHold, SIGNAL(valueChanged(int)), this, SLOT(setHold(int)));
    }
}

void InstrumentPropertiesDialog::setName(const QString &name)
{
    song->instrument(instrument)->setName(name);
}

void InstrumentPropertiesDialog::setMidiChannel(int midiChannel)
{
    song->instrument(instrument)->setMidiChannel(midiChannel);
}

void InstrumentPropertiesDialog::setVolume(int volume)
{
    song->instrument(instrument)->setDefaultVelocity(volume);
}

void InstrumentPropertiesDialog::setTranspose(int transpose)
{
    song->instrument(instrument)->setTranspose(transpose);
}

void InstrumentPropertiesDialog::setHold(int hold)
{
    song->instrument(instrument)->setHold(hold);
}
