/*
 * alsamidiinterface.cpp
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

#include "alsamidi.h"
#include "alsamidiinterface.h"

AlsaMIDIInterface::AlsaMIDIInterface(AlsaMIDI *midi, snd_seq_port_info_t *pinfo, DirectionFlags flags, QObject *parent) :
    MIDIInterface(flags, parent),
    midi(midi),
    client(snd_seq_port_info_get_client(pinfo)),
    port(snd_seq_port_info_get_port(pinfo)),
    subs(NULL)
{
    name_ = snd_seq_port_info_get_name(pinfo);

    snd_seq_addr_t sender, dest;

    if ((flags & Output) != 0) {
        sender.client = midi->client;
        sender.port = midi->port;
        dest.client = client;
        dest.port = port;
    } else {
        sender.client = client;
        sender.port = port;
        dest.client = midi->client;
        dest.port = midi->port;
    }
    snd_seq_port_subscribe_malloc(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
}

AlsaMIDIInterface::~AlsaMIDIInterface()
{
    snd_seq_port_subscribe_free(subs);
}

void AlsaMIDIInterface::write(const QByteArray &data)
{
    // Create event
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, midi->port);
    snd_seq_ev_set_dest(&ev, client, port);
    snd_seq_ev_set_direct(&ev);

    // The encoder may send the data in multiple packets
    for (int sent = 0; sent < data.length();) {
        sent += snd_midi_event_encode(midi->encoder, (const unsigned char *)(data.constData() + sent), data.length() - sent, &ev);
        snd_seq_event_output(midi->seq, &ev);
    }
    snd_seq_drain_output(midi->seq);
}

void AlsaMIDIInterface::setEnabled(bool enabled)
{
    bool wasEnabled = isEnabled();

    MIDIInterface::setEnabled(enabled);

    if (enabled != wasEnabled) {
        if (enabled) {
            snd_seq_subscribe_port(midi->seq, subs);
        } else {
            snd_seq_unsubscribe_port(midi->seq, subs);
        }
    }
}
