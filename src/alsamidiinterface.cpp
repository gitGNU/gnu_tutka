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
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);

    // Try subscribing to the port
    snd_seq_subscribe_port(midi->seq, subs);
}

void AlsaMIDIInterface::write(const char *data, unsigned int length)
{
    // Create event
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_source(&ev, midi->port);
    snd_seq_ev_set_dest(&ev, client, port);
    snd_seq_ev_set_direct(&ev);

    // The encoder may send the data in multiple packets
    for (int sent = 0; sent < length;) {
        sent += snd_midi_event_encode(midi->encoder, (const unsigned char *)(data + sent), length - sent, &ev);
        snd_seq_event_output(midi->seq, &ev);
    }
    snd_seq_drain_output(midi->seq);
}
