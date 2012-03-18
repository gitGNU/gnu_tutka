/*
 * preferencesdialog.cpp
 *
 * Copyright 2002-2012 Vesa Halttunen
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

#include "player.h"
#include "midi.h"
#include "midiinterface.h"
#include "outputmidiinterfacestablemodel.h"
#include "inputmidiinterfacestablemodel.h"
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(Player *player, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    player(player),
    outputMidiInterfacesTableModel(new OutputMidiInterfacesTableModel(player->midi())),
    inputMidiInterfacesTableModel(new InputMidiInterfacesTableModel(player->midi())),
    settings("nongnu.org", "Tutka")
{
    ui->setupUi(this);

    ui->tableViewOutputMidiInterfaces->setModel(outputMidiInterfacesTableModel);
    ui->tableViewOutputMidiInterfaces->setColumnWidth(0, 300);
    ui->tableViewInputMidiInterfaces->setModel(inputMidiInterfacesTableModel);
    ui->tableViewInputMidiInterfaces->setColumnWidth(0, 300);

    connect(ui->comboBoxSchedulingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setSchedulingMode(int)));
    loadSettings();

    connect(player->midi(), SIGNAL(inputsChanged()), this, SLOT(saveSettings()));
    connect(player->midi(), SIGNAL(outputsChanged()), this, SLOT(saveSettings()));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::setSchedulingMode(int schedulingMode)
{
    player->setScheduler(schedulingMode == 0 ? Player::SchedulingRTC : Player::SchedulingNanoSleep);

    saveSettings();
}

void PreferencesDialog::loadSettings()
{
    MIDI *midi = player->midi();
    QStringList inputs;
    QStringList outputs;
    QStringList unavailableInputs;
    QStringList unavailableOutputs;

    QString inputsString = settings.value("MIDI/In").toString();
    if (!inputsString.isEmpty()) {
        foreach (const QString &inputName, inputsString.split(",")) {
            int input = midi->input(inputName);
            if (input >= 0) {
                midi->input(input)->setEnabled(true);
                inputs.append(inputName);
            } else {
                unavailableInputs.append(inputName);
            }
        }
    }

    QString outputsString = settings.value("MIDI/Out").toString();
    if (!outputsString.isEmpty()) {
        foreach (const QString &outputName, outputsString.split(",")) {
            int output = midi->output(outputName);
            if (output >= 0) {
                midi->output(output)->setEnabled(true);
                outputs.append(outputName);
            } else {
                unavailableOutputs.append(outputName);
            }
        }
    }

    QString unavailableInputsString = settings.value("MIDI/UnavailableIn").toString();
    if (!unavailableInputsString.isEmpty()) {
        foreach (const QString &inputName, unavailableInputsString.split(",")) {
            int input = midi->input(inputName);
            if (input >= 0) {
                midi->input(input)->setEnabled(true);
                inputs.append(inputName);
            } else {
                unavailableInputs.append(inputName);
            }
        }
    }

    QString unavailableOutputsString = settings.value("MIDI/UnavailableOut").toString();
    if (!unavailableOutputsString.isEmpty()) {
        foreach (const QString &outputName, unavailableOutputsString.split(",")) {
            int output = midi->output(outputName);
            if (output >= 0) {
                midi->output(output)->setEnabled(true);
                outputs.append(outputName);
            } else {
                unavailableOutputs.append(outputName);
            }
        }
    }

    settings.setValue("MIDI/In", inputs.join(","));
    settings.setValue("MIDI/Out", outputs.join(","));
    settings.setValue("MIDI/UnavailableIn", unavailableInputs.join(","));
    settings.setValue("MIDI/UnavailableOut", unavailableOutputs.join(","));
    ui->comboBoxSchedulingMode->setCurrentIndex(settings.value("SchedulingMode").toInt());
}

void PreferencesDialog::saveSettings()
{
    MIDI *midi = player->midi();
    QStringList inputs;
    QStringList outputs;

    for (int input = 0; input < midi->inputs(); input++) {
        MIDIInterface *interface = midi->input(input).data();
        if (interface->isEnabled()) {
            inputs.append(interface->name());
        }
    }

    for (int output = 1; output < midi->outputs(); output++) {
        MIDIInterface *interface = midi->output(output).data();
        if (interface->isEnabled()) {
            outputs.append(interface->name());
        }
    }

    settings.setValue("MIDI/In", inputs.join(","));
    settings.setValue("MIDI/Out", outputs.join(","));
    settings.setValue("SchedulingMode", ui->comboBoxSchedulingMode->currentIndex());
}
