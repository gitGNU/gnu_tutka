#include "coremidiinterface.h"

CoreMIDIInterface::CoreMIDIInterface(MIDIEndpointRef endpoint, DirectionFlags flags, QObject *parent) :
    MIDIInterface(flags, parent)
{
    name_ = getMidiDeviceName(endpoint);
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

