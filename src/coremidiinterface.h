#ifndef COREMIDIINTERFACE_H
#define COREMIDIINTERFACE_H

#include <MIDIServices.h>
#include "midiinterface.h"

class CoreMIDI;

class CoreMIDIInterface : public MIDIInterface
{
public:
    CoreMIDIInterface(CoreMIDI *midi, MIDIEndpointRef endpoint, DirectionFlags flags, QObject *parent = NULL);

    virtual void write(const QByteArray &data);
    virtual void setEnabled(bool enabled);

private:
    CoreMIDI *midi;
    MIDIEndpointRef endpoint;

    static QString getMidiDeviceName(MIDIEndpointRef endpoint);
};

#endif // COREMIDIINTERFACE_H
