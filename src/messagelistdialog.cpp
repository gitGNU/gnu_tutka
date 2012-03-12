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
    spinBoxDelegate(new SpinBoxDelegate(this)),
    selectedMessage(-1)
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
    connect(ui->tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(setSelection(QItemSelection, QItemSelection)));
    connect(ui->tableView->model(), SIGNAL(modelReset()), this, SLOT(setSelection()));

    setSelection();
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
    song->insertMessage(selectedMessage >= 0 ? selectedMessage : 0);
}

void MessageListDialog::appendMessage()
{
    song->insertMessage(song->messages());
}

void MessageListDialog::deleteMessage()
{
    if (selectedMessage >= 0) {
        song->deleteMessage(selectedMessage);
    }
}

void MessageListDialog::sendMessage()
{
    if (selectedMessage >= 0) {
        Message *message = song->message(selectedMessage);
        for (int output = 0; output < midi->outputs(); output++) {
            midi->output(output)->writeRaw(message->data());
        }
    }
}

void MessageListDialog::receiveMessage()
{
    connect(midi, SIGNAL(inputReceived(QByteArray)), this, SLOT(receiveMessage(QByteArray)), Qt::UniqueConnection);
}

void MessageListDialog::loadMessage()
{
    if (selectedMessage >= 0) {
        QString path = QFileDialog::getOpenFileName(NULL, tr("Load message"));

        if (!path.isEmpty()) {
            song->message(selectedMessage)->loadBinary(path);
        }
    }
}

void MessageListDialog::saveMessage()
{
    if (selectedMessage >= 0) {
        QString path = QFileDialog::getSaveFileName(NULL, tr("Save message as"));

        if (!path.isEmpty()) {
            song->message(selectedMessage)->saveBinary(path);
        }
    }
}

void MessageListDialog::setSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    bool messageSelected = !indexes.isEmpty();
    ui->pushButtonDelete->setEnabled(messageSelected);
    ui->pushButtonSend->setEnabled(messageSelected);
    ui->pushButtonReceive->setEnabled(messageSelected);
    ui->pushButtonLoad->setEnabled(messageSelected);
    ui->pushButtonSave->setEnabled(messageSelected);
    selectedMessage = messageSelected ? indexes.first().row() : -1;
}

void MessageListDialog::receiveMessage(const QByteArray &data)
{
    if (selectedMessage < 0 || data.isEmpty() || data[0] == (char)0xfe) {
        return;
    }

    receivedMessage.append(data);
    if (receivedMessage.length() >= song->message(selectedMessage)->length()) {
        receivedMessage.truncate(song->message(selectedMessage)->length());
        song->message(selectedMessage)->setData(receivedMessage);

        receivedMessage.clear();
        disconnect(midi, SIGNAL(inputReceived(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
    }
}
