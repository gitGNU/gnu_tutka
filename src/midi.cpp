/*
 * midi.cpp
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

#include "midiinterface.h"
#include "midi.h"

MIDI::MIDI()
{
    updateInterfaces();
}

MIDI::~MIDI()
{
}

QSharedPointer<MIDIInterface> MIDI::interface(unsigned int number) const
{
    return interfaces_.at(number);
}

unsigned int MIDI::interfaces() const
{
    return interfaces_.count();
}

void MIDI::updateInterfaces()
{
    interfaces_.clear();
    interfaces_.append(QSharedPointer<MIDIInterface>(new MIDIInterface(MIDIInterface::Input | MIDIInterface::Output)));
}
