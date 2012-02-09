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
        // Disconnect the widgets from any previously selected instrument
        Instrument *oldInstrument = song->instrument(this->instrument);
        disconnect(oldInstrument, SIGNAL(nameChanged(QString)), ui->lineEditName, SLOT(setText(QString)));
        disconnect(ui->lineEditName, SIGNAL(textChanged(QString)), oldInstrument, SLOT(setName(QString)));
        disconnect(ui->horizontalSliderMidiChannel, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setMidiChannel(int)));
        disconnect(ui->horizontalSliderVolume, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setDefaultVelocity(int)));
        disconnect(ui->horizontalSliderTranspose, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setTranspose(int)));
        disconnect(ui->horizontalSliderHold, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setHold(int)));

        // Make sure the instrument exists
        this->instrument = number;
        song->checkInstrument(this->instrument);

        // Connect the widgets for editing the instrument
        Instrument *instrument = song->instrument(this->instrument);
        connect(instrument, SIGNAL(nameChanged(QString)), ui->lineEditName, SLOT(setText(QString)));
        connect(ui->lineEditName, SIGNAL(textChanged(QString)), instrument, SLOT(setName(QString)));
        connect(ui->horizontalSliderMidiChannel, SIGNAL(valueChanged(int)), instrument, SLOT(setMidiChannel(int)));
        connect(ui->horizontalSliderVolume, SIGNAL(valueChanged(int)), instrument, SLOT(setDefaultVelocity(int)));
        connect(ui->horizontalSliderTranspose, SIGNAL(valueChanged(int)), instrument, SLOT(setTranspose(int)));
        connect(ui->horizontalSliderHold, SIGNAL(valueChanged(int)), instrument, SLOT(setHold(int)));

        // Show the instrument's properties in the UI
        ui->lineEditName->blockSignals(true);
        ui->lineEditName->setText(instrument->name());
        ui->lineEditName->blockSignals(false);
        ui->horizontalSliderMidiChannel->blockSignals(true);
        ui->horizontalSliderMidiChannel->setValue(instrument->midiChannel() + 1);
        ui->horizontalSliderMidiChannel->blockSignals(false);
        ui->horizontalSliderVolume->blockSignals(true);
        ui->horizontalSliderVolume->setValue(instrument->defaultVelocity());
        ui->horizontalSliderVolume->blockSignals(false);
        ui->horizontalSliderTranspose->blockSignals(true);
        ui->horizontalSliderTranspose->setValue(instrument->transpose());
        ui->horizontalSliderTranspose->blockSignals(false);
        ui->horizontalSliderHold->blockSignals(true);
        ui->horizontalSliderHold->setValue(instrument->hold());
        ui->horizontalSliderHold->blockSignals(false);
    }
}
