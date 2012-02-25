/*
 * messagelistdialog.cpp
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

#include <QFileDialog>
#include "midi.h"
#include "midiinterface.h"
#include "song.h"
#include "spinboxdelegate.h"
#include "messagelisttablemodel.h"
#include "messagelistdialog.h"
#include "ui_messagelistdialog.h"

MessageListDialog::MessageListDialog(MIDI *midi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageListDialog),
    midi(midi),
    song(NULL),
    messageListTableModel(new MessageListTableModel(this)),
    spinBoxDelegate(new SpinBoxDelegate(this))
{
    ui->setupUi(this);

    ui->tableView->setModel(messageListTableModel);
    ui->tableView->setColumnWidth(0, 200);
    ui->tableView->setItemDelegateForColumn(1, spinBoxDelegate);

    connect(ui->pushButtonInsertNew, SIGNAL(clicked()), this, SLOT(insertMessage()));
    connect(ui->pushButtonAppendNew, SIGNAL(clicked()), this, SLOT(appendMessage()));
    connect(ui->pushButtonDelete, SIGNAL(clicked()), this, SLOT(deleteMessage()));
    connect(ui->pushButtonSend, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(ui->pushButtonReceive, SIGNAL(clicked()), this, SLOT(receiveMessage()));
    connect(ui->pushButtonLoad, SIGNAL(clicked()), this, SLOT(loadMessage()));
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(saveMessage()));
}

MessageListDialog::~MessageListDialog()
{
    delete ui;
}

void MessageListDialog::setSong(Song *song)
{
    this->song = song;
    messageListTableModel->setSong(song);
}

void MessageListDialog::insertMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertMessage(indexes.first().row());
    }
}

void MessageListDialog::appendMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->insertMessage(song->messages());
    }
}

void MessageListDialog::deleteMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        song->deleteMessage(indexes.first().row());
    }
}

void MessageListDialog::sendMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        Message *message = song->message(indexes.first().row());
        for (int output = 0; output < midi->outputs(); output++) {
            midi->output(output)->writeRaw(message->data());
        }
    }
}

void MessageListDialog::receiveMessage()
{
}

void MessageListDialog::loadMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        QString path = QFileDialog::getOpenFileName();

        if (!path.isEmpty()) {
            song->message(indexes.first().row())->loadBinary(path);
        }
    }
}

void MessageListDialog::saveMessage()
{
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        QString path = QFileDialog::getSaveFileName();

        if (!path.isEmpty()) {
            song->message(indexes.first().row())->saveBinary(path);
        }
    }
}
