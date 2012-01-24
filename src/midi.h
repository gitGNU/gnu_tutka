/*
 * midi.h
 *
 * Copyright 2002-2011 vesuri
 *
 * This file is part of Tutka2.
 *
 * Tutka2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka2; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MIDI_H
#define _MIDI_H

#include <QList>

class Message;

class MIDIInterface
{
public:
    MIDIInterface();
    virtual ~MIDIInterface();

    /* Sets the current tick */
    virtual void setTick(unsigned int) = 0;
    /* Stops a note playing on a MIDI channel using requested velocity */
    virtual void noteOff(unsigned char, unsigned char, unsigned char) = 0;
    /* Plays a note on a MIDI channel using requested velocity */
    virtual void noteOn(unsigned char, unsigned char, unsigned char) = 0;
    /* Sets the aftertouch pressure of a note playing on a MIDI channel */
    virtual void aftertouch(unsigned char, unsigned char, unsigned char) = 0;
    /* Sets the MIDI controller value of a MIDI channel */
    virtual void controller(unsigned char, unsigned char, unsigned char) = 0;
    /* Send a program change on a MIDI channel */
    virtual void programChange(unsigned char, unsigned char) = 0;
    /* Sets the channel pressure of a MIDI channel */
    virtual void channelPressure(unsigned char, unsigned char) = 0;
    /* Sets the pitch wheel value of a MIDI channel */
    virtual void pitchWheel(unsigned char, unsigned short) = 0;
    /* Sends a MIDI message */
    virtual void writeRaw(const char *, unsigned short) = 0;
    /* Send a clock message */
    virtual void clock() = 0;
    /* Set the tempo (used when exporting) */
    virtual void tempo(unsigned int) = 0;
};

class MIDI
{
public:
    /* Open the MIDI subsystem */
    MIDI();
    virtual ~MIDI();

    MIDIInterface *output(unsigned int) const;
    unsigned int outputs() const;
    /* Get MIDI interface from available interfaces by name */
    MIDIInterface *midiInterface(char *, unsigned int);
    /* Gets an MIDI interface number by name from the available interfaces */
    int midiInterfaceNumber(char *, unsigned int);
    /* Gets an MIDI interface name by number from the available interfaces */
    char *midiInterfaceName(unsigned int, unsigned int);
    /* Opens the MIDI interfaces */
    void subscribeAll();
    /* Unsubscribe all MIDI interfaces and free data */
    void unsubscribeAll();
    /* Gets the list of available interfaces */
    void interfaces(unsigned int);
    /* Refreshes the list of available interfaces */
    void refreshInterfaces();
    /* Receives a MIDI message */
    unsigned char *readRaw();
    /* Receives a MIDI message */
    int readSystemExclusive(Message *, unsigned int, int);
    /* Returns the data written to the MIDI buffer */
    unsigned char *buffer() const;
    /* Returns the length of data written to the MIDI buffer */
    int bufferLength() const;

private:
    QList<MIDIInterface *> outputs_;
};

#endif // _MIDI_H
