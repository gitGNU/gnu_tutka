#include "coremidiinterface.h"
#include "coremidi.h"

CoreMIDI::CoreMIDI(QObject *parent) :
    MIDI(parent)
{
    MIDIClientCreate(CFSTR("Tutka"), handleMidiNotification, this, &client);
    MIDIOutputPortCreate(client, CFSTR("Tutka Output"), &outputPort);
    MIDIInputPortCreate(client, CFSTR("Tutka Input"), readMidi, this, &inputPort);
    MIDISourceCreate(client, CFSTR("Tutka Output"), &source);

    updateInterfaces();
}

CoreMIDI::~CoreMIDI()
{
    MIDIEndpointDispose(source);
    MIDIPortDispose(outputPort);
    MIDIPortDispose(inputPort);
    MIDIClientDispose(client);
}

void CoreMIDI::updateInterfaces()
{
    MIDI::updateInterfaces();

    for (ItemCount index = 0; index < MIDIGetNumberOfDestinations(); index++) {
        MIDIInterface *interface = new CoreMIDIInterface(this, MIDIGetDestination(index), MIDIInterface::Output);
        connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(outputEnabledChanged(bool)));
        outputs_.append(QSharedPointer<MIDIInterface>(interface));
    }

    for (ItemCount index = 0; index < MIDIGetNumberOfSources(); index++) {
        MIDIEndpointRef source = MIDIGetSource(index);
        if (source != this->source) {
            MIDIInterface *interface = new CoreMIDIInterface(this, source, MIDIInterface::Input);
            connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(inputEnabledChanged(bool)));
            connect(interface, SIGNAL(inputReceived(QByteArray)), this, SIGNAL(inputReceived(QByteArray)));
            inputs_.append(QSharedPointer<MIDIInterface>(interface));
        }
    }

    emit outputsChanged();
    emit inputsChanged();
}


void CoreMIDI::readMidi(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
{
    Q_UNUSED(srcConnRefCon)

    CoreMIDI *midi = (CoreMIDI *)readProcRefCon;
    for (int packet = 0; packet < pktlist->numPackets; packet++) {
        emit midi->inputReceived(QByteArray((const char *)pktlist->packet[packet].data, pktlist->packet[packet].length));
    }
}

void CoreMIDI::handleMidiNotification(const MIDINotification *message, void *refCon)
{
    if (message->messageID == kMIDIMsgSetupChanged) {
        CoreMIDI *midi = (CoreMIDI *)refCon;
        midi->updateInterfaces();
    }
}
