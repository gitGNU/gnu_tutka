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
    midi(midi),
    ui(new Ui::MessageListDialog),
    settings("nongnu.org", "Tutka"),
    song(NULL),
    messageListTableModel(new MessageListTableModel(this)),
    selectedMessage(-1)
{
    ui->setupUi(this);

    ui->tableView->setModel(messageListTableModel);
    ui->tableView->setColumnWidth(0, 200);
    ui->tableView->setItemDelegateForColumn(1, new SpinBoxDelegate(0, 16777216, this));

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

void MessageListDialog::makeVisible()
{
    show();
    raise();
    activateWindow();
}

void MessageListDialog::setSong(Song *song)
{
    this->song = song;
    selectedMessage = -1;
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
    if (selectedMessage >= 0 && song->message(selectedMessage)->length() > 0) {
        disconnect(ui->pushButtonReceive, SIGNAL(clicked()), this, SLOT(receiveMessage()));
        connect(ui->pushButtonReceive, SIGNAL(clicked()), this, SLOT(stopReception()));
        connect(midi, SIGNAL(inputReceived(QByteArray)), this, SLOT(receiveMessage(QByteArray)), Qt::UniqueConnection);
        ui->pushButtonReceive->setText(tr("Stop"));
        ui->labelStatus->setText(tr("%1/%2 bytes received").arg(0).arg(song->message(selectedMessage)->length()));
    }
}

void MessageListDialog::loadMessage()
{
    if (selectedMessage >= 0) {
        QString path = QFileDialog::getOpenFileName(NULL, tr("Load message"), settings.value("Paths/messagePath").toString());

        if (!path.isEmpty()) {
            song->message(selectedMessage)->loadBinary(path);

            settings.setValue("Paths/messagePath", QFileInfo(path).absolutePath());
        }
    }
}

void MessageListDialog::saveMessage()
{
    if (selectedMessage >= 0) {
        QString path = QFileDialog::getSaveFileName(NULL, tr("Save message as"), settings.value("Paths/messagePath").toString());

        if (!path.isEmpty()) {
            song->message(selectedMessage)->saveBinary(path);

            settings.setValue("Paths/messagePath", QFileInfo(path).absolutePath());
        }
    }
}

void MessageListDialog::setSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    if (selectedMessage >= 0 && selectedMessage < song->messages()) {
        disconnect(song->message(selectedMessage), SIGNAL(lengthChanged()), this, SLOT(setReceiveButtonVisibility()));
    }
    QModelIndexList indexes = ui->tableView->selectionModel()->selectedIndexes();
    bool messageSelected = !indexes.isEmpty();
    selectedMessage = messageSelected ? indexes.first().row() : -1;
    ui->pushButtonDelete->setEnabled(messageSelected);
    ui->pushButtonSend->setEnabled(messageSelected);
    ui->pushButtonLoad->setEnabled(messageSelected);
    ui->pushButtonSave->setEnabled(messageSelected);
    setReceiveButtonVisibility();
    if (selectedMessage >= 0) {
        connect(song->message(selectedMessage), SIGNAL(lengthChanged()), this, SLOT(setReceiveButtonVisibility()));
    }
}

void MessageListDialog::receiveMessage(const QByteArray &data)
{
    if (selectedMessage < 0 || data.isEmpty() || data[0] == (char)0xfe) {
        return;
    }

    receivedMessage.append(data);
    ui->labelStatus->setText(tr("%1/%2 bytes received").arg(receivedMessage.length()).arg(song->message(selectedMessage)->length()));

    if (receivedMessage.length() >= song->message(selectedMessage)->length()) {
        receivedMessage.truncate(song->message(selectedMessage)->length());
        song->message(selectedMessage)->setData(receivedMessage);

        stopReception();
    }
}

void MessageListDialog::setReceiveButtonVisibility()
{
    ui->pushButtonReceive->setEnabled(selectedMessage >= 0 && song->message(selectedMessage)->length() > 0);
}

void MessageListDialog::stopReception()
{
    receivedMessage.clear();

    disconnect(midi, SIGNAL(inputReceived(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
    disconnect(ui->pushButtonReceive, SIGNAL(clicked()), this, SLOT(stopReception()));
    connect(ui->pushButtonReceive, SIGNAL(clicked()), this, SLOT(receiveMessage()));
    ui->pushButtonReceive->setText(tr("Receive"));
    ui->labelStatus->setText(QString());
}
