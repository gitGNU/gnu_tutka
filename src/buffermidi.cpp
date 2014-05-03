/*
 * buffermidi.cpp
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

#include "buffermidi.h"
#include "buffermidiinterface.h"

BufferMIDI::BufferMIDI(QObject *parent) : MIDI(parent)
{
    updateInterfaces();
}

BufferMIDI::~BufferMIDI()
{
}

void BufferMIDI::updateInterfaces()
{
    outputs_.clear();
    outputs_.append(QSharedPointer<MIDIInterface>(new BufferMIDIInterface(this, MIDIInterface::Output)));
    inputs_.clear();
    inputs_.append(QSharedPointer<MIDIInterface>(new BufferMIDIInterface(this, MIDIInterface::Input)));
}

QByteArray BufferMIDI::data() const
{
    return static_cast<BufferMIDIInterface *>(outputs_[0].data())->data();
}
