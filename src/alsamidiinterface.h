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

    virtual QByteArray read();
    virtual void write(const QByteArray &data);

private:
    AlsaMIDI *midi;
    int client;
    int port;
    snd_seq_port_subscribe_t *subs;
    QList<QByteArray> input;
};

#endif // ALSAMIDIINTERFACE_H
