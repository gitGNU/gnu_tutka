/*
 * coremidiinterface.h
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

#ifndef COREMIDIINTERFACE_H
#define COREMIDIINTERFACE_H

#include <MIDIServices.h>
#include "midiinterface.h"

class CoreMIDI;

class CoreMIDIInterface : public MIDIInterface
{
public:
    CoreMIDIInterface(CoreMIDI *midi, MIDIEndpointRef endpoint, DirectionFlags flags, QObject *parent = NULL);

    virtual void write(const QByteArray &data);
    virtual void setEnabled(bool enabled);

private:
    CoreMIDI *midi;
    MIDIEndpointRef endpoint;

    static QString getMidiDeviceName(MIDIEndpointRef endpoint);
};

#endif // COREMIDIINTERFACE_H
