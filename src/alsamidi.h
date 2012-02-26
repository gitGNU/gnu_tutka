#ifndef ALSAMIDI_H
#define ALSAMIDI_H

#include <alsa/asoundlib.h>
#include "midi.h"

class AlsaMIDI : public MIDI
{
    Q_OBJECT

public:
    explicit AlsaMIDI(QObject *parent = NULL);
    virtual ~AlsaMIDI();

protected:
    virtual void updateInterfaces();

private:
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

    friend class AlsaMIDIInterface;
};

#endif // ALSAMIDI_H
