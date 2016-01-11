/*
 * coremidiinterface.cpp
 *
 * Copyright 2002-2016 Vesa Halttunen
 *
 * This file is part of Tutka.
 *
 * Tutka is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "coremidi.h"
#include "coremidiinterface.h"

CoreMIDIInterface::CoreMIDIInterface(CoreMIDI *midi, MIDIEndpointRef endpoint, DirectionFlags flags, QObject *parent) :
    MIDIInterface(flags, parent),
    midi(midi),
    endpoint(endpoint)
{
    name_ = getMidiDeviceName(endpoint);
}

void CoreMIDIInterface::write(const QByteArray &data)
{
    MIDIPacketList list;
    list.numPackets = 1;
    list.packet[0].timeStamp = 0;
    list.packet[0].length = data.length();
    memcpy(list.packet[0].data, data.constData(), data.length());

    MIDISend(midi->outputPort, endpoint, &list);
    MIDIReceived(midi->source, &list);
}

void CoreMIDIInterface::setEnabled(bool enabled)
{
    bool wasEnabled = isEnabled();

    MIDIInterface::setEnabled(enabled);

    if (enabled != wasEnabled && (flags_ & Input) != 0) {
        if (enabled) {
            MIDIPortConnectSource(midi->inputPort, endpoint, midi);
        } else {
            MIDIPortDisconnectSource(midi->inputPort, endpoint);
        }
    }
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
