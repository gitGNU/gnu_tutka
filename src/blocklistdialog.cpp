/*
 * blocklistdialog.cpp
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

#include "song.h"
#include "spinboxdelegate.h"
#include "blocklisttablemodel.h"
#include "blocklistdialog.h"
#include "ui_blocklistdialog.h"

BlockListDialog::BlockListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockListDialog),
    song(NULL),
    blockListTableModel(new BlockListTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(blockListTableModel);
    ui->tableView->setColumnWidth(0, 200);
    ui->tableView->setColumnWidth(3, 100);
    ui->tableView->setItemDelegateForColumn(1, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(2, spinBoxDelegate);
    ui->tableView->setItemDelegateForColumn(3, spinBoxDelegate);

    connect(ui->pushButtonInsertNew, SIGNAL(clicked()), this, SLOT(insertBlock()));
    connect(ui->pushButtonAppendNew, SIGNAL(clicked()), this, SLOT(appendBlock()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteBlock()));
}

BlockListDialog::~BlockListDialog()
{
    delete ui;
}

void BlockListDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void BlockListDialog::setSong(Song *song)
{
    this->song = song;
    blockListTableModel->setSong(song);
}

void BlockListDialog::setBlock(unsigned int block)
{
    ui->tableView->selectRow(block);
}

void BlockListDialog::insertBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertBlock(indexes.first().row(), indexes.first().row());
    }
}

void BlockListDialog::appendBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertBlock(song->blocks(), indexes.first().row());
    }
}

void BlockListDialog::deleteBlock()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deleteBlock(indexes.first().row());
    }
}
