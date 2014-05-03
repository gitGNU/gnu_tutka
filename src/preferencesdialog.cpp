/*
 * preferencesdialog.cpp
 *
 * Copyright 2002-2014 Vesa Halttunen
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
#include "scheduler.h"
#include "midi.h"
#include "midiinterface.h"
#include "outputmidiinterfacestablemodel.h"
#include "inputmidiinterfacestablemodel.h"
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(Player *player, QWidget *parent) :
    QDialog(parent),
    player(player),
    ui(new Ui::PreferencesDialog),
    settings("nongnu.org", "Tutka"),
    outputMidiInterfacesTableModel(new OutputMidiInterfacesTableModel(player->midi())),
    inputMidiInterfacesTableModel(new InputMidiInterfacesTableModel(player->midi()))
{
    ui->setupUi(this);

    ui->tableViewOutputMidiInterfaces->setModel(outputMidiInterfacesTableModel);
    ui->tableViewOutputMidiInterfaces->setColumnWidth(0, 300);
    ui->tableViewInputMidiInterfaces->setModel(inputMidiInterfacesTableModel);
    ui->tableViewInputMidiInterfaces->setColumnWidth(0, 300);

    enableInterfaces();

    int index = 0;
    foreach (Scheduler *scheduler, Scheduler::schedulers()) {
        ui->comboBoxSchedulingMode->insertItem(index, tr(scheduler->name()));
        schedulers.insert(index++, scheduler);
    }
    setSchedulingMode(settings.value("schedulingMode").toString());

    connect(player->midi(), SIGNAL(outputsChanged()), this, SLOT(enableInterfaces()));
    connect(player->midi(), SIGNAL(inputsChanged()), this, SLOT(enableInterfaces()));
    connect(ui->comboBoxSchedulingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setScheduler(int)));
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void PreferencesDialog::setSchedulingMode(const QString &name)
{
    int currentIndex = 0;

    for (int index = 0; index < schedulers.count(); index++) {
        Scheduler *scheduler = schedulers.value(index);
        if (scheduler->name() == name) {
            currentIndex = index;
            break;
        }
    }

    ui->comboBoxSchedulingMode->setCurrentIndex(currentIndex);
    player->setScheduler(schedulers.value(currentIndex));
}

void PreferencesDialog::setScheduler(int index)
{
    Scheduler *scheduler = schedulers.value(index);
    player->setScheduler(scheduler);
    settings.setValue("schedulingMode", scheduler->name());
}

void PreferencesDialog::enableInterfaces()
{
    MIDI *midi = player->midi();
    disconnect(midi, SIGNAL(outputEnabledChanged(bool)), this, SLOT(saveSettings()));
    disconnect(midi, SIGNAL(inputEnabledChanged(bool)), this, SLOT(saveSettings()));

    QStringList unavailableInputs;
    QStringList unavailableOutputs;

    QString unavailableInputsString = settings.value("MIDI/unavailableInputInterfaces").toString();
    if (!unavailableInputsString.isEmpty()) {
        foreach (const QString &inputName, unavailableInputsString.split(",")) {
            int input = midi->input(inputName);
            if (input >= 0) {
                midi->input(input)->setEnabled(true);
            } else {
                unavailableInputs.append(inputName);
            }
        }
    }

    QString inputsString = settings.value("MIDI/inputInterfaces").toString();
    if (!inputsString.isEmpty()) {
        foreach (const QString &inputName, inputsString.split(",")) {
            int input = midi->input(inputName);
            if (input >= 0) {
                midi->input(input)->setEnabled(true);
            } else {
                unavailableInputs.append(inputName);
            }
        }
    }

    QString unavailableOutputsString = settings.value("MIDI/unavailableOutputInterfaces").toString();
    if (!unavailableOutputsString.isEmpty()) {
        foreach (const QString &outputName, unavailableOutputsString.split(",")) {
            int output = midi->output(outputName);
            if (output >= 0) {
                midi->output(output)->setEnabled(true);
            } else {
                unavailableOutputs.append(outputName);
            }
        }
    }

    QString outputsString = settings.value("MIDI/outputInterfaces").toString();
    if (!outputsString.isEmpty()) {
        foreach (const QString &outputName, outputsString.split(",")) {
            int output = midi->output(outputName);
            if (output >= 0) {
                midi->output(output)->setEnabled(true);
            } else {
                unavailableOutputs.append(outputName);
            }
        }
    }

    settings.setValue("MIDI/unavailableInputInterfaces", unavailableInputs.join(","));
    settings.setValue("MIDI/unavailableOutputInterfaces", unavailableOutputs.join(","));

    saveSettings();

    connect(midi, SIGNAL(outputEnabledChanged(bool)), this, SLOT(saveSettings()));
    connect(midi, SIGNAL(inputEnabledChanged(bool)), this, SLOT(saveSettings()));
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

    settings.setValue("MIDI/inputInterfaces", inputs.join(","));
    settings.setValue("MIDI/outputInterfaces", outputs.join(","));
}
