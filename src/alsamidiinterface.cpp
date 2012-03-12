#include <QTimer>
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

    QTimer::singleShot(0, this, SLOT(read()));
}

void AlsaMIDIInterface::read()
{
    snd_seq_event_t *ev;
    while (snd_seq_event_input(midi->seq, &ev) >= 0) {
        // Check event type
        qWarning("XX TYPE %d", ev->type);
        switch (ev->type) {
        // TODO support ALSA sequencer events
        case SND_SEQ_EVENT_START:
//            if (editor_player_get_external_sync(midi->editor) == EXTERNAL_SYNC_MIDI)
//                editor_player_start(midi->editor, MODE_PLAY_SONG, 0);
            break;
        case SND_SEQ_EVENT_CONTINUE:
//            if (editor_player_get_external_sync(midi->editor) == EXTERNAL_SYNC_MIDI)
//                editor_player_start(midi->editor, MODE_PLAY_SONG, 1);
            break;
        case SND_SEQ_EVENT_STOP:
//            if (editor_player_get_external_sync(midi->editor) == EXTERNAL_SYNC_MIDI)
//                editor_player_stop(midi->editor);
            break;
        case SND_SEQ_EVENT_CLOCK:
//            if (editor_player_get_external_sync(midi->editor) == EXTERNAL_SYNC_MIDI)
//                editor_player_external_sync(midi->editor, 1);
            break;
        case SND_SEQ_EVENT_PORT_START:
        case SND_SEQ_EVENT_PORT_EXIT:
        case SND_SEQ_EVENT_PORT_CHANGE:
            // Ports have been changed
//            midi->changed = 1;
            break;
        default: {
            // Get the event to the incoming buffer
            QByteArray data(snd_seq_event_length(ev), 0);
            int decoded = snd_midi_event_decode(midi->decoder, (unsigned char *)data.constData(), data.length(), ev);
            if (decoded < 0) {
                // Some sort of an error occurred
                break;
            }

            qWarning("XX PERKELE %d", data.length());
            emit inputReceived(data);
        }
        }
    }

    QTimer::singleShot(250, this, SLOT(read()));
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
