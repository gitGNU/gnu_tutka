#ifndef BUFFERMIDIINTERFACE_H
#define BUFFERMIDIINTERFACE_H

#include "midiinterface.h"

class BufferMIDI;

class BufferMIDIInterface : public MIDIInterface
{
    Q_OBJECT

public:
    explicit BufferMIDIInterface(BufferMIDI *midi, DirectionFlags flags, QObject *parent = NULL);
    QByteArray data() const;

protected:
    virtual void write(const char *data, unsigned int length);

private:
    BufferMIDI *midi;
    QByteArray data_;
};

#endif // BUFFERMIDIINTERFACE_H
