/*
 * midi.cpp
 *
 * Copyright 2002-2012 vesuri
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

#include "midiinterface.h"
#include "midi.h"

MIDI::MIDI()
{
    outputs_.append(QSharedPointer<MIDIInterface>(new MIDIInterface));
}

MIDI::~MIDI()
{
}

QSharedPointer<MIDIInterface> MIDI::output(unsigned int number) const
{
    return outputs_.at(number);
}

unsigned int MIDI::outputs() const
{
    return outputs_.count();
}
