#ifndef COREMIDIINTERFACE_H
#define COREMIDIINTERFACE_H

#include <MIDIServices.h>
#include "midiinterface.h"

class CoreMIDIInterface : public MIDIInterface
{
public:
    CoreMIDIInterface(MIDIClientRef client, MIDIEndpointRef endpoint, DirectionFlags flags, QObject *parent = NULL);

    virtual void write(const unsigned char *data, unsigned int length);

private:
    MIDIClientRef client;
    MIDIEndpointRef endpoint;
    MIDIPortRef outputPort;
    MIDIPortRef inputPort;

    static QString getMidiDeviceName(MIDIEndpointRef endpoint);
};

#endif // COREMIDIINTERFACE_H
