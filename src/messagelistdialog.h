/*
 * messagelistdialog.h
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

#ifndef MESSAGELISTDIALOG_H
#define MESSAGELISTDIALOG_H

#include <QDialog>
#include <QItemSelection>

namespace Ui {
    class MessageListDialog;
}

class MIDI;
class Song;
class MessageListTableModel;
class SpinBoxDelegate;

class MessageListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageListDialog(MIDI *midi, QWidget *parent = 0);
    ~MessageListDialog();

public slots:
    void setSong(Song *song);

private slots:
    void insertMessage();
    void appendMessage();
    void deleteMessage();
    void sendMessage();
    void receiveMessage();
    void loadMessage();
    void saveMessage();
    void setSelection(const QItemSelection &selected = QItemSelection(), const QItemSelection &deselected = QItemSelection());

private:
    Ui::MessageListDialog *ui;
    MIDI *midi;
    Song *song;
    MessageListTableModel *messageListTableModel;
    SpinBoxDelegate *spinBoxDelegate;
};

#endif // MESSAGELISTDIALOG_H
