/*
 * midiinterface.h
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

#ifndef MIDIINTERFACE_H
#define MIDIINTERFACE_H

#include <QObject>
#include <QString>

class QByteArray;

class MIDIInterface : public QObject
{
    Q_OBJECT

public:
    enum DirectionFlag {
        Input = 0x1,
        Output = 0x2
    };
    Q_DECLARE_FLAGS(DirectionFlags, DirectionFlag)

    MIDIInterface(DirectionFlags flags, QObject *parent = NULL);
    virtual ~MIDIInterface();

    // Returns the name of the interface
    QString name() const;

    // Returns the flags for the interface
    DirectionFlags flags() const;

    // Whether the interface is enabled or not
    bool isEnabled() const;

    // Sets the current tick
    void setTick(unsigned int);

    // Stops a note playing on a MIDI channel using requested velocity
    void noteOff(unsigned char, unsigned char, unsigned char);

    // Plays a note on a MIDI channel using requested velocity
    void noteOn(unsigned char, unsigned char, unsigned char);

    // Sets the aftertouch pressure of a note playing on a MIDI channel
    void aftertouch(unsigned char, unsigned char, unsigned char);

    // Sets the MIDI controller value of a MIDI channel
    void controller(unsigned char, unsigned char, unsigned char);

    // Send a program change on a MIDI channel
    void programChange(unsigned char, unsigned char);

    // Sets the channel pressure of a MIDI channel
    void channelPressure(unsigned char, unsigned char);

    // Sets the pitch wheel value of a MIDI channel
    void pitchWheel(unsigned char, unsigned short);

    // Sends a MIDI message
    void writeRaw(const QByteArray &data);

    // Send a clock message
    void clock();

    // Set the tempo (used when exporting)
    void tempo(unsigned int);

protected:
    virtual void write(const QByteArray &data);

public slots:
    // Enables or disables the interface
    virtual void setEnabled(bool enabled);

signals:
    // Emitted when the interface is enabled or disabled
    void enabledChanged(bool);

    // Emitted when input has been received
    void inputReceived(QByteArray data);

protected:
    QString name_;
    DirectionFlags flags_;
    bool enabled;
    unsigned int tick;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MIDIInterface::DirectionFlags)

#endif // MIDIINTERFACE_H
