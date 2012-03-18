#include "coremidiinterface.h"
#include "coremidi.h"

CoreMIDI::CoreMIDI(QObject *parent) :
    MIDI(parent)
{
    MIDIClientCreate(CFSTR("Tutka"), NULL, NULL, &client);

    updateInterfaces();
}

void CoreMIDI::updateInterfaces()
{
    MIDI::updateInterfaces();

    for (ItemCount index = 0; index < MIDIGetNumberOfDestinations(); index++) {
        MIDIInterface *interface = new CoreMIDIInterface(client, MIDIGetDestination(index), MIDIInterface::Output);
        connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(outputEnabledChanged(bool)));
        outputs_.append(QSharedPointer<MIDIInterface>(interface));
    }

    for (ItemCount index = 0; index < MIDIGetNumberOfSources(); index++) {
        MIDIInterface *interface = new CoreMIDIInterface(client, MIDIGetSource(index), MIDIInterface::Input);
        connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(inputEnabledChanged(bool)));
        connect(interface, SIGNAL(inputReceived(QByteArray)), this, SIGNAL(inputReceived(QByteArray)));
        inputs_.append(QSharedPointer<MIDIInterface>(interface));
    }

    emit outputsChanged();
    emit inputsChanged();
}
