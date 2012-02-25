#include <QByteArray>
#include <cstdio>
#include "midiinterface.h"

MIDIInterface::MIDIInterface()
{
}

MIDIInterface::~MIDIInterface()
{
}

void MIDIInterface::setTick(unsigned int tick)
{
    printf("Tick %d\n", tick);
}

void MIDIInterface::noteOff(unsigned char channel, unsigned char note, unsigned char velocity)
{
    printf("Note off %d %d %d\n", channel, note, velocity);
}

void MIDIInterface::noteOn(unsigned char channel, unsigned char note, unsigned char velocity)
{
    printf("Note on %d %d %d\n", channel, note, velocity);
}

void MIDIInterface::aftertouch(unsigned char channel, unsigned char note, unsigned char pressure)
{
    printf("Aftertouch %d %d %d\n", channel, note, pressure);
}

void MIDIInterface::controller(unsigned char channel, unsigned char controller, unsigned char value)
{
    printf("Controller %d %d %d\n", channel, controller, value);
}

void MIDIInterface::programChange(unsigned char channel, unsigned char program)
{
    printf("Program change %d %d\n", channel, program);
}

void MIDIInterface::channelPressure(unsigned char channel, unsigned char pressure)
{
    printf("Channel presure %d %d\n", channel, pressure);
}

void MIDIInterface::pitchWheel(unsigned char channel, unsigned short value)
{
    printf("Pitch wheel %d %d\n", channel, value);
}

void MIDIInterface::writeRaw(const QByteArray &data)
{
    printf("Write raw %p %d\n", data.constData(), data.length());
}

void MIDIInterface::clock()
{
    printf("Clock\n");
}

void MIDIInterface::tempo(unsigned int tempo)
{
    printf("Tempo %d\n", tempo);
}
