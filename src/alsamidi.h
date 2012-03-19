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
