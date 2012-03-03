#include "buffermidi.h"
#include "buffermidiinterface.h"

BufferMIDIInterface::BufferMIDIInterface(BufferMIDI *midi, DirectionFlags flags, QObject *parent) :
    MIDIInterface(flags, parent),
    midi(midi)
{
}

void BufferMIDIInterface::write(const unsigned char *data, unsigned int length)
{
    data_.append((const char *)data, length);
}

QByteArray BufferMIDIInterface::data() const
{
    return data_;
}
