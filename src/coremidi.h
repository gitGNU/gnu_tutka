#ifndef COREMIDI_H
#define COREMIDI_H

#include <MIDIServices.h>
#include "midi.h"

class CoreMIDI : public MIDI
{
public:
    CoreMIDI(QObject *parent = NULL);

protected:
    virtual void updateInterfaces();

private:
    static void handleMidiNotification(const MIDINotification *message, void *refCon);

    MIDIClientRef client;
};

#endif // COREMIDI_H
