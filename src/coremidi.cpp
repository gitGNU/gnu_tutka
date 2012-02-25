#include "coremidiinterface.h"
#include "coremidi.h"

CoreMIDI::CoreMIDI()
{
    updateInterfaces();
}

void CoreMIDI::updateInterfaces()
{
    MIDI::updateInterfaces();

    for (ItemCount index = 0; index < MIDIGetNumberOfSources(); index++) {
        interfaces_.append(QSharedPointer<MIDIInterface>(new CoreMIDIInterface(MIDIGetSource(index), MIDIInterface::Input)));
    }
    for (ItemCount index = 0; index < MIDIGetNumberOfDestinations(); index++) {
        interfaces_.append(QSharedPointer<MIDIInterface>(new CoreMIDIInterface(MIDIGetDestination(index), MIDIInterface::Output)));
    }

    foreach (const QSharedPointer<MIDIInterface> &output, interfaces_) {
        qWarning("%s", output->name().toUtf8().constData());
    }
}
