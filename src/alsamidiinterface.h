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

    virtual void write(const unsigned char *data, unsigned int length);

private:
    AlsaMIDI *midi;
    int client;
    int port;
    snd_seq_port_subscribe_t *subs;
};

#endif // ALSAMIDIINTERFACE_H