#ifndef COREMIDIINTERFACE_H
#define COREMIDIINTERFACE_H

#include <MIDIServices.h>
#include "midiinterface.h"

class CoreMIDIInterface : public MIDIInterface
{
public:
    CoreMIDIInterface(MIDIClientRef client, MIDIEndpointRef endpoint, DirectionFlags flags, QObject *parent = NULL);

    virtual QByteArray read();
    virtual void write(const QByteArray &data);

private:
    static void readMidi(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon);

    MIDIClientRef client;
    MIDIEndpointRef endpoint;
    MIDIPortRef outputPort;
    MIDIPortRef inputPort;
    QList<QByteArray> input;

    static QString getMidiDeviceName(MIDIEndpointRef endpoint);
};

#endif // COREMIDIINTERFACE_H
