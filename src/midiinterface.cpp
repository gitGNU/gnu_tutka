/*
 * midiinterface.cpp
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

#include <QByteArray>
#include "midiinterface.h"

MIDIInterface::MIDIInterface(DirectionFlags flags, QObject *parent) :
    QObject(parent),
    name_("No output"),
    flags_(flags),
    enabled(true)
{
}

MIDIInterface::~MIDIInterface()
{
}

QString MIDIInterface::name() const
{
    return name_;
}

MIDIInterface::DirectionFlags MIDIInterface::flags() const
{
    return flags_;
}

bool MIDIInterface::isEnabled() const
{
    return enabled;
}

void MIDIInterface::setEnabled(bool enabled)
{
    this->enabled = enabled;

    emit enabledChanged(enabled);
}

void MIDIInterface::setTick(unsigned int tick)
{
    qDebug("Tick %d", tick);
}

void MIDIInterface::noteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    qDebug("Note off %d %d %d", channel, note, velocity);
}

void MIDIInterface::noteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    qDebug("Note on %d %d %d", channel, note, velocity);
}

void MIDIInterface::aftertouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    qDebug("Aftertouch %d %d %d", channel, note, pressure);
}

void MIDIInterface::controller(unsigned char channel, unsigned char controller, unsigned char value)
{
    qDebug("Controller %d %d %d", channel, controller, value);
}

void MIDIInterface::programChange(unsigned char channel, unsigned char program)
{
    qDebug("Program change %d %d", channel, program);
}

void MIDIInterface::channelPressure(unsigned char channel, unsigned char pressure)
{
    qDebug("Channel presure %d %d", channel, pressure);
}

void MIDIInterface::pitchWheel(unsigned char channel, unsigned short value)
{
    qDebug("Pitch wheel %d %d", channel, value);
}

void MIDIInterface::writeRaw(const QByteArray &data)
{
    qDebug("Write raw %p %d", data.constData(), data.length());
}

void MIDIInterface::clock()
{
    qDebug("Clock");
}

void MIDIInterface::tempo(unsigned int tempo)
{
    qDebug("Tempo %d", tempo);
}
