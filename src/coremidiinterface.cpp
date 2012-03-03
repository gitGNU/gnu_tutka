#include "coremidiinterface.h"

CoreMIDIInterface::CoreMIDIInterface(MIDIClientRef client, MIDIEndpointRef endpoint, DirectionFlags flags, QObject *parent) :
    MIDIInterface(flags, parent),
    client(client),
    endpoint(endpoint),
    outputPort(NULL),
    inputPort(NULL)
{
    name_ = getMidiDeviceName(endpoint);

    if ((flags & Output) != 0) {
        MIDIOutputPortCreate(client, CFSTR("Tutka Output"), &outputPort);
    }

    if ((flags & Input) != 0) {
//        MIDIInputPortCreate(client, CFSTR("Tutka Input"), &inputPort);
    }
//    setEnabled(false);
}

QString CoreMIDIInterface::getMidiDeviceName(MIDIEndpointRef endpoint)
{
    MIDIEntityRef entity;
    MIDIDeviceRef device;

    CFStringRef endpointName = NULL;
    CFStringRef deviceName = NULL;
    CFStringRef fullName = NULL;
    CFStringEncoding defaultEncoding = CFStringGetSystemEncoding();

    MIDIEndpointGetEntity(endpoint, &entity);
    MIDIEntityGetDevice(entity, &device);

    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &endpointName);
    MIDIObjectGetStringProperty(device, kMIDIPropertyName, &deviceName);

    if (deviceName != NULL) {
        fullName = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@: %@"), deviceName, endpointName);
    } else {
        fullName = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@"), endpointName);
    }

    char *newName = new char[CFStringGetLength(fullName) + 1];

    CFStringGetCString(fullName, newName, CFStringGetLength(fullName) + 1, defaultEncoding);

    if (endpointName != NULL) {
        CFRelease(endpointName);
    }
    if (deviceName) {
        CFRelease(deviceName);
    }
    if (fullName) {
        CFRelease(fullName);
    }

    QString name(newName);
    delete [] newName;
    return name;
}

void CoreMIDIInterface::write(const char *data, unsigned int length)
{
    MIDIPacketList list;
    list.numPackets = 1;
    list.packet[0].timeStamp = 0;
    list.packet[0].length = length;
    memcpy(list.packet[0].data, data, length);

    MIDISend(outputPort, endpoint, &list);
}
