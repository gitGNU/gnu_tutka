#include <QTimer>
#include "alsamidiinterface.h"
#include "alsamidi.h"

AlsaMIDI::AlsaMIDI(QObject *parent) :
    MIDI(parent),
    seq(NULL),
    client(0),
    port(0),
    encoder(NULL),
    decoder(NULL),
    inputThread(this)
{
    snd_seq_addr_t sender, dest;
    snd_seq_port_subscribe_t *subs;

    int err = snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
    if (err < 0) {
        qWarning("Couldn't create ALSA MIDI client: %s", snd_strerror(err));
    } else {
        snd_seq_set_client_name(seq, "Tutka");
        client = snd_seq_client_id(seq);

        // Create a new port
        port = snd_seq_create_simple_port(seq, "Tutka", SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
        if (port < 0) {
            qWarning("Couldn't create port: %s", snd_strerror(err));
        } else {
            // Subscribe to the announce port
            sender.client = SND_SEQ_CLIENT_SYSTEM;
            sender.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
            dest.client = client;
            dest.port = port;
            snd_seq_port_subscribe_alloca(&subs);
            snd_seq_port_subscribe_set_sender(subs, &sender);
            snd_seq_port_subscribe_set_dest(subs, &dest);
            snd_seq_subscribe_port(seq, subs);

            // Create a MIDI event encoder and decoder
            snd_midi_event_new(65536, &encoder);
            snd_midi_event_new(65536, &decoder);
            snd_midi_event_init(encoder);
            snd_midi_event_init(decoder);
            snd_midi_event_no_status(encoder, 1);
            snd_midi_event_no_status(decoder, 1);

            updateInterfaces();
        }
    }

    inputThread.start();
}

AlsaMIDI::~AlsaMIDI()
{
    if (seq != NULL) {
        snd_seq_addr_t sender, dest;
        snd_seq_port_subscribe_t *subs;

        // Unsubscribe the announce port
        sender.client = SND_SEQ_CLIENT_SYSTEM;
        sender.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
        dest.client = client;
        dest.port = port;
        snd_seq_port_subscribe_alloca(&subs);
        snd_seq_port_subscribe_set_sender(subs, &sender);
        snd_seq_port_subscribe_set_dest(subs, &dest);
        snd_seq_unsubscribe_port(seq, subs);

        // Unsubscribe and free all interfaces
        inputs_.clear();
        outputs_.clear();

        // Free MIDI event encoder and decoder
        if (encoder != NULL) {
            snd_midi_event_free(encoder);
        }
        if (decoder != NULL) {
            snd_midi_event_free(decoder);
        }

        // Delete port
        if (port >= 0) {
            snd_seq_delete_simple_port(seq, port);
        }

        // Delete client
        snd_seq_close(seq);
        seq = NULL;
    }
}

void AlsaMIDI::updateInterfaces()
{
    MIDI::updateInterfaces();

    snd_seq_client_info_t *cinfo;
    snd_seq_client_info_alloca(&cinfo);
    snd_seq_port_info_t *pinfo;
    snd_seq_port_info_alloca(&pinfo);
    snd_seq_client_info_set_client(cinfo, SND_SEQ_CLIENT_SYSTEM);
    while (snd_seq_query_next_client(seq, cinfo) >= 0) {
        // Reset query info
        snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
        snd_seq_port_info_set_port(pinfo, -1);
        while (snd_seq_query_next_port(seq, pinfo) >= 0) {
            int client = snd_seq_port_info_get_client(pinfo);
            int port = snd_seq_port_info_get_port(pinfo);

            // Check whether the capabilities match the current direction
            bool isOutput = (snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) != 0;
            bool isInput = (snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)) != 0;
            if ((isOutput || isInput) && !(client == this->client && port == this->port)) {
                // Create an interface structure for the port
                if (isOutput) {
                    MIDIInterface *interface = new AlsaMIDIInterface(this, pinfo, MIDIInterface::Output);
                    connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(outputEnabledChanged(bool)));
                    outputs_.append(QSharedPointer<MIDIInterface>(interface));
                }

                if (isInput) {
                    MIDIInterface *interface = new AlsaMIDIInterface(this, pinfo, MIDIInterface::Input);
                    connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(inputEnabledChanged(bool)));
                    connect(interface, SIGNAL(inputReceived(QByteArray)), this, SIGNAL(inputReceived(QByteArray)));
                    connect(interface, SIGNAL(startReceived()), this, SIGNAL(startReceived()));
                    connect(interface, SIGNAL(stopReceived()), this, SIGNAL(stopReceived()));
                    connect(interface, SIGNAL(continueReceived()), this, SIGNAL(continueReceived()));
                    connect(interface, SIGNAL(clockReceived()), this, SIGNAL(clockReceived()));
                    inputs_.append(QSharedPointer<MIDIInterface>(interface));
                }
            }
        }
    }

    emit outputsChanged();
    emit inputsChanged();
}

AlsaMIDI::InputThread::InputThread(AlsaMIDI *midi) :
        QThread(midi),
        midi(midi)
{
}

void AlsaMIDI::InputThread::run()
{
    while (midi->seq != NULL) {
        int pollDescriptorCount = snd_seq_poll_descriptors_count(midi->seq, POLLIN);
        struct pollfd pollDescriptors[pollDescriptorCount];
        snd_seq_poll_descriptors(midi->seq, pollDescriptors, pollDescriptorCount, POLLIN);

        if (poll(pollDescriptors, pollDescriptorCount, -1) > 0) {
            snd_seq_event_t *ev;
            while (snd_seq_event_input(midi->seq, &ev) >= 0) {
                switch (ev->type) {
                case SND_SEQ_EVENT_START:
                    emit midi->startReceived();
                    break;
                case SND_SEQ_EVENT_CONTINUE:
                    emit midi->continueReceived();
                    break;
                case SND_SEQ_EVENT_STOP:
                    emit midi->stopReceived();
                    break;
                case SND_SEQ_EVENT_CLOCK:
                    emit midi->clockReceived();
                    break;
                case SND_SEQ_EVENT_PORT_START:
                case SND_SEQ_EVENT_PORT_EXIT:
                case SND_SEQ_EVENT_PORT_CHANGE:
                    // Ports have been added, removed or changed so update interfaces
                    midi->updateInterfaces();
                    break;
                default: {
                    // Get the event to the incoming buffer and decode it
                    int length = snd_seq_event_length(ev);
                    unsigned char temp[length];
                    int decoded = snd_midi_event_decode(midi->decoder, temp, length, ev);
                    if (decoded >= 0) {
                        emit midi->inputReceived(QByteArray((const char*)(temp), decoded));
                    }
                    break;
                }
                }
            }
        }
    }
}
