/*
 * midi.cpp
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

#include "midiinterface.h"
#include "midi.h"

MIDI::MIDI(QObject *parent) :
    QObject(parent)
{
    updateInterfaces();
}

MIDI::~MIDI()
{
}

QSharedPointer<MIDIInterface> MIDI::output(unsigned int number) const
{
    return outputs_.at(number);
}

int MIDI::output(const QString &name) const
{
    for (int output = 0; output < outputs_.count(); output++) {
        if (outputs_[output]->name() == name) {
            return output;
        }
    }

    return -1;
}

unsigned int MIDI::outputs() const
{
    return outputs_.count();
}

QSharedPointer<MIDIInterface> MIDI::input(unsigned int number) const
{
    return inputs_.at(number);
}

int MIDI::input(const QString &name) const
{
    for (int input = 0; input < inputs_.count(); input++) {
        if (inputs_[input]->name() == name) {
            return input;
        }
    }

    return -1;
}

unsigned int MIDI::inputs() const
{
    return inputs_.count();
}

void MIDI::updateInterfaces()
{
    MIDIInterface *output = new MIDIInterface(MIDIInterface::Output);
    output->setEnabled(true);

    outputs_.clear();
    outputs_.append(QSharedPointer<MIDIInterface>(output));
    inputs_.clear();
}
