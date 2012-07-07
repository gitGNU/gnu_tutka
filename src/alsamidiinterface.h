/*
 * alsamidiinterface.h
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

#ifndef ALSAMIDIINTERFACE_H
#define ALSAMIDIINTERFACE_H

#include <alsa/asoundlib.h>
#include "midiinterface.h"

class AlsaMIDI;

class AlsaMIDIInterface : public MIDIInterface
{
    Q_OBJECT

public:
    explicit AlsaMIDIInterface(AlsaMIDI *midi, snd_seq_port_info_t *pinfo, DirectionFlags flags, QObject *parent = NULL);
    virtual ~AlsaMIDIInterface();

    virtual void write(const QByteArray &data);
    virtual void setEnabled(bool enabled);

signals:
    void startReceived();
    void stopReceived();
    void continueReceived();
    void clockReceived();

private:
    AlsaMIDI *midi;
    int client;
    int port;
    snd_seq_port_subscribe_t *subs;
};

#endif // ALSAMIDIINTERFACE_H
