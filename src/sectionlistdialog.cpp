/*
 * sectionlistdialog.cpp
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

#include "song.h"
#include "spinboxdelegate.h"
#include "sectionlisttablemodel.h"
#include "sectionlistdialog.h"
#include "ui_sectionlistdialog.h"

SectionListDialog::SectionListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SectionListDialog),
    song(NULL),
    sectionListTableModel(new SectionListTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(1, 1, this)),
    section(-1)
{
    ui->setupUi(this);

    ui->tableView->setModel(sectionListTableModel);
    ui->tableView->setItemDelegateForColumn(0, spinBoxDelegate);

    connect(ui->pushButtonInsert, SIGNAL(clicked()), this, SLOT(insertSection()));
    connect(ui->pushButtonAppend, SIGNAL(clicked()), this, SLOT(appendSection()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteSection()));
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(setSection(QItemSelection, QItemSelection)));
    connect(sectionListTableModel, SIGNAL(modelReset()), this, SLOT(setSelection()));
}

SectionListDialog::~SectionListDialog()
{
    delete ui;
}

void SectionListDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void SectionListDialog::setSong(Song *song)
{
    if (this->song != NULL) {
        disconnect(this->song, SIGNAL(playseqsChanged(int)), spinBoxDelegate, SLOT(setMaximum(int)));
        disconnect(this->song, SIGNAL(sectionsChanged(unsigned int)), this, SLOT(setDeleteButtonVisibility()));
    }

    this->song = song;
    connect(this->song, SIGNAL(playseqsChanged(int)), spinBoxDelegate, SLOT(setMaximum(int)));
    sectionListTableModel->setSong(song);

    connect(song, SIGNAL(sectionsChanged(unsigned int)), this, SLOT(setDeleteButtonVisibility()));
    setDeleteButtonVisibility();
}

void SectionListDialog::setSection(unsigned int section)
{
    ui->tableView->selectRow(section);
    this->section = section;
}

void SectionListDialog::insertSection()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertSection(indexes.first().row());
    }
}

void SectionListDialog::appendSection()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertSection(song->sections());
    }
}

void SectionListDialog::deleteSection()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deleteSection(indexes.first().row());
    }
}


void SectionListDialog::setSelection()
{
    if (section >= 0) {
        ui->tableView->selectRow(section);
    }
}

void SectionListDialog::setSection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    QModelIndexList indexes = selected.indexes();
    if (!indexes.isEmpty()) {
        int section = indexes.first().row();
        if (section != this->section) {
            this->section = section;
            emit sectionSelected(this->section);
        }
    }
}

void SectionListDialog::setDeleteButtonVisibility()
{
    ui->pushButtonDelete->setEnabled(song != NULL && song->sections() > 1);
}
