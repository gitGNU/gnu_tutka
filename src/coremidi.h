#ifndef COREMIDI_H
#define COREMIDI_H

#include <MIDIServices.h>
#include "midi.h"

class CoreMIDI : public MIDI
{
public:
    CoreMIDI(QObject *parent = NULL);
    virtual ~CoreMIDI();

protected:
    virtual void updateInterfaces();

private:
    static void readMidi(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon);
    static void handleMidiNotification(const MIDINotification *message, void *refCon);

    MIDIClientRef client;
    MIDIPortRef outputPort;
    MIDIPortRef inputPort;
    MIDIEndpointRef source;

    friend class CoreMIDIInterface;
};

#endif // COREMIDI_H
