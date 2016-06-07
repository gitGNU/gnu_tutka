/*
 * midiinterface.cpp
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

#include <QByteArray>
#include "midiinterface.h"

MIDIInterface::MIDIInterface(DirectionFlags flags, QObject *parent) :
    QObject(parent),
    name_("No output"),
    flags_(flags),
    enabled(false),
    tick(0)
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
//    qDebug("Tick %d", tick);

    this->tick = tick;
}

void MIDIInterface::noteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    qDebug("Note off %d %d %d", channel, note, velocity);

    QByteArray data(3, 0);

    data[0] = 0x80 | channel;
    data[1] = note & 0x7f;
    data[2] = velocity & 0x7f;

    writeRaw(data);
}

void MIDIInterface::noteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    qDebug("Note on %d %d %d", channel, note, velocity);

    QByteArray data(3, 0);

    data[0] = 0x90 | channel;
    data[1] = note & 0x7f;
    data[2] = velocity & 0x7f;

    writeRaw(data);
}

void MIDIInterface::aftertouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    qDebug("Aftertouch %d %d %d", channel, note, pressure);

    QByteArray data(3, 0);

    data[0] = 0xa0 | channel;
    data[1] = note & 0x7f;
    data[2] = pressure & 0x7f;

    writeRaw(data);
}

void MIDIInterface::controller(unsigned char channel, unsigned char controller, unsigned char value)
{
    qDebug("Controller %d %d %d", channel, controller, value);

    QByteArray data(3, 0);

    data[0] = 0xb0 | channel;
    data[1] = controller & 0x7f;
    data[2] = value & 0x7f;

    writeRaw(data);
}

void MIDIInterface::programChange(unsigned char channel, unsigned char program)
{
    qDebug("Program change %d %d", channel, program);

    QByteArray data(2, 0);

    data[0] = 0xc0 | channel;
    data[1] = program & 0x7f;

    writeRaw(data);
}

void MIDIInterface::channelPressure(unsigned char channel, unsigned char pressure)
{
    qDebug("Channel pressure %d %d", channel, pressure);

    QByteArray data(2, 0);

    data[0] = 0xd0 | channel;
    data[1] = pressure & 0x7f;

    writeRaw(data);
}

void MIDIInterface::pitchWheel(unsigned char channel, unsigned short value)
{
    qDebug("Pitch wheel %d %d", channel, value);

    QByteArray data(3, 0);

    data[0] = 0xe0 | channel;
    data[1] = (value >> 7) & 0x7f;
    data[2] = value & 0x7f;

    writeRaw(data);
}

void MIDIInterface::writeRaw(const QByteArray &data)
{
    qDebug("Write raw %p %d", data.constData(), data.length());

    if (enabled && !data.isEmpty()) {
        write(data);
    }
}

void MIDIInterface::clock()
{
    qDebug("Clock");

    QByteArray data(1, 0xf8);

    writeRaw(data);
}

void MIDIInterface::start()
{
    qDebug("Start");

    QByteArray data(1, 0xfa);

    writeRaw(data);
}

void MIDIInterface::cont()
{
    qDebug("Continue");

    QByteArray data(1, 0xfb);

    writeRaw(data);
}

void MIDIInterface::stop()
{
    qDebug("Stop");

    QByteArray data(1, 0xfc);

    writeRaw(data);
}

void MIDIInterface::tempo(unsigned int tempo)
{
    qDebug("Tempo %d", tempo);
}

void MIDIInterface::write(const QByteArray &data)
{
    qDebug("Write %d bytes", data.length());
}
