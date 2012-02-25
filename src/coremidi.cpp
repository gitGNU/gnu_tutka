#include "coremidiinterface.h"
#include "coremidi.h"

CoreMIDI::CoreMIDI(QObject *parent) :
    MIDI(parent)
{
    updateInterfaces();
}

void CoreMIDI::updateInterfaces()
{
    MIDI::updateInterfaces();

    for (ItemCount index = 0; index < MIDIGetNumberOfDestinations(); index++) {
        MIDIInterface *interface = new CoreMIDIInterface(MIDIGetDestination(index), MIDIInterface::Output);
        connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(outputsChanged()));
        outputs_.append(QSharedPointer<MIDIInterface>(interface));
    }
    for (ItemCount index = 0; index < MIDIGetNumberOfSources(); index++) {
        MIDIInterface *interface = new CoreMIDIInterface(MIDIGetSource(index), MIDIInterface::Input);
        connect(interface, SIGNAL(enabledChanged(bool)), this, SIGNAL(inputsChanged()));
        inputs_.append(QSharedPointer<MIDIInterface>(interface));
    }
}
