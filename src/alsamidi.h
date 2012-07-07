/*
 * alsamidi.h
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

#ifndef ALSAMIDI_H
#define ALSAMIDI_H

#include <QThread>
#include <alsa/asoundlib.h>
#include "midi.h"

class AlsaMIDI : public MIDI
{
    Q_OBJECT

public:
    explicit AlsaMIDI(QObject *parent = NULL);
    virtual ~AlsaMIDI();

protected slots:
    virtual void updateInterfaces();

private:
    class InputThread : public QThread
    {
    public:
        InputThread(AlsaMIDI *midi);
        virtual void run();

    private:
        AlsaMIDI *midi;
    };

    // ALSA MIDI sequencer interface
    snd_seq_t *seq;

    // Sequencer client ID
    int client;

    // Sequencer port ID
    int port;

    // MIDI event encoder
    snd_midi_event_t *encoder;

    // MIDI event decoder
    snd_midi_event_t *decoder;

    // Input thread
    InputThread inputThread;

    friend class AlsaMIDIInterface;
};

#endif // ALSAMIDI_H
