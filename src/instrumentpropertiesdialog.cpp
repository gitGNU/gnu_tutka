/*
 * instrumentpropertiesdialog.cpp
 *
 * Copyright 2002-2016 Vesa Halttunen
 *
 * This file is part of Tutka.
 *
 * Tutka is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "midi.h"
#include "midiinterface.h"
#include "song.h"
#include "instrumentpropertiesdialog.h"
#include "ui_instrumentpropertiesdialog.h"

InstrumentPropertiesDialog::InstrumentPropertiesDialog(MIDI *midi, QWidget *parent) :
    QDialog(parent),
    midi(midi),
    ui(new Ui::InstrumentPropertiesDialog),
    song(NULL),
    instrument(0)
{
    ui->setupUi(this);

    connect(ui->comboBoxMidiInterface, SIGNAL(currentIndexChanged(QString)), this, SLOT(setMidiInterface(QString)));
    connect(midi, SIGNAL(outputsChanged()), this, SLOT(updateMidiInterfaceComboBox()));
    connect(midi, SIGNAL(outputEnabledChanged(bool)), this, SLOT(updateMidiInterfaceComboBox()));
    updateMidiInterfaceComboBox();
}

InstrumentPropertiesDialog::~InstrumentPropertiesDialog()
{
    delete ui;
}

void InstrumentPropertiesDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void InstrumentPropertiesDialog::setSong(Song *song)
{
    this->song = song;
}

void InstrumentPropertiesDialog::setInstrument(int number)
{
    if (number >= 0) {
        // Disconnect the widgets from any previously selected instrument
        Instrument *oldInstrument = song->instrument(this->instrument);
        disconnect(oldInstrument, SIGNAL(nameChanged(QString)), ui->lineEditName, SLOT(setText(QString)));
        disconnect(ui->lineEditName, SIGNAL(textChanged(QString)), oldInstrument, SLOT(setName(QString)));
        disconnect(ui->horizontalSliderMidiChannel, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setMidiChannel(int)));
        disconnect(ui->horizontalSliderVolume, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setDefaultVelocity(int)));
        disconnect(ui->horizontalSliderTranspose, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setTranspose(int)));
        disconnect(ui->horizontalSliderHold, SIGNAL(valueChanged(int)), oldInstrument, SLOT(setHold(int)));
        disconnect(ui->comboBoxMidiInterface, SIGNAL(currentIndexChanged(QString)), oldInstrument, SLOT(setMidiInterfaceName(QString)));

        // Make sure the instrument exists
        this->instrument = number;
        song->checkInstrument(this->instrument);

        // Show the instrument's properties in the UI
        Instrument *instrument = song->instrument(this->instrument);
        ui->lineEditName->setText(instrument->name());
        ui->horizontalSliderMidiChannel->setValue(instrument->midiChannel() + 1);
        ui->horizontalSliderVolume->setValue(instrument->defaultVelocity());
        ui->horizontalSliderTranspose->setValue(instrument->transpose());
        ui->horizontalSliderHold->setValue(instrument->hold());
        updateMidiInterfaceComboBox();

        // Connect the widgets for editing the instrument
        connect(instrument, SIGNAL(nameChanged(QString)), ui->lineEditName, SLOT(setText(QString)));
        connect(ui->lineEditName, SIGNAL(textChanged(QString)), instrument, SLOT(setName(QString)));
        connect(ui->horizontalSliderMidiChannel, SIGNAL(valueChanged(int)), this, SLOT(setMidiChannel(int)));
        connect(ui->horizontalSliderVolume, SIGNAL(valueChanged(int)), instrument, SLOT(setDefaultVelocity(int)));
        connect(ui->horizontalSliderTranspose, SIGNAL(valueChanged(int)), instrument, SLOT(setTranspose(int)));
        connect(ui->horizontalSliderHold, SIGNAL(valueChanged(int)), instrument, SLOT(setHold(int)));
        connect(ui->comboBoxMidiInterface, SIGNAL(currentIndexChanged(QString)), instrument, SLOT(setMidiInterfaceName(QString)));
    }
}

void InstrumentPropertiesDialog::updateMidiInterfaceComboBox()
{
    ui->comboBoxMidiInterface->blockSignals(true);
    ui->comboBoxMidiInterface->clear();

    for (int number = 0; number < midi->outputs(); number++) {
        QSharedPointer<MIDIInterface> interface = midi->output(number);
        if (interface->isEnabled()) {
            ui->comboBoxMidiInterface->addItem(midi->output(number)->name());
            if (song != NULL && midi->output(number)->name() == song->instrument(this->instrument)->midiInterfaceName()) {
                ui->comboBoxMidiInterface->setCurrentIndex(ui->comboBoxMidiInterface->count() - 1);
            }
        }
    }
    ui->comboBoxMidiInterface->blockSignals(false);
}

void InstrumentPropertiesDialog::setMidiInterface(const QString &name)
{
    if (song != NULL) {
        song->instrument(instrument)->setMidiInterface(midi->output(name));
    }
}

void InstrumentPropertiesDialog::setMidiChannel(int midiChannel)
{
    song->instrument(instrument)->setMidiChannel(midiChannel - 1);
}
