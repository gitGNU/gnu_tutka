/*
 * midi.h
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

#ifndef _MIDI_H
#define _MIDI_H

#include <QObject>
#include <QSharedPointer>
#include <QList>

class MIDIInterface;
class Message;

class MIDI : public QObject
{
    Q_OBJECT

public:
    // Open the MIDI subsystem
    MIDI(QObject *parent = NULL);
    virtual ~MIDI();

    QSharedPointer<MIDIInterface> output(unsigned int) const;
    int output(const QString &name) const;
    unsigned int outputs() const;

    QSharedPointer<MIDIInterface> input(unsigned int) const;
    int input(const QString &name) const;
    unsigned int inputs() const;

signals:
    void outputsChanged();
    void inputsChanged();
    void outputEnabledChanged(bool enabled);
    void inputEnabledChanged(bool enabled);
    void startReceived();
    void stopReceived();
    void continueReceived();
    void clockReceived();

    // Emitted when input has been received
    void inputReceived(QByteArray data);

protected:
    virtual void updateInterfaces();

    QList<QSharedPointer<MIDIInterface> > outputs_;
    QList<QSharedPointer<MIDIInterface> > inputs_;
};

#endif // _MIDI_H
