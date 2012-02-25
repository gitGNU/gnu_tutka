#ifndef COREMIDI_H
#define COREMIDI_H

#include "midi.h"

class CoreMIDI : public MIDI
{
public:
    CoreMIDI();

protected:
    virtual void updateInterfaces();
};

#endif // COREMIDI_H
