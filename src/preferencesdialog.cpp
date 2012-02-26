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

#include "midi.h"
#include "outputmidiinterfacestablemodel.h"
#include "inputmidiinterfacestablemodel.h"
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(MIDI *midi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    outputMidiInterfacesTableModel(new OutputMidiInterfacesTableModel(midi)),
    inputMidiInterfacesTableModel(new InputMidiInterfacesTableModel(midi))
{
    ui->setupUi(this);

    ui->tableViewOutputMidiInterfaces->setModel(outputMidiInterfacesTableModel);
    ui->tableViewOutputMidiInterfaces->setColumnWidth(0, 300);
    ui->tableViewInputMidiInterfaces->setModel(inputMidiInterfacesTableModel);
    ui->tableViewInputMidiInterfaces->setColumnWidth(0, 300);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}
