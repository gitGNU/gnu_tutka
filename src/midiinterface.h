#ifndef MIDIINTERFACE_H
#define MIDIINTERFACE_H

class QByteArray;

class MIDIInterface
{
public:
    MIDIInterface();
    virtual ~MIDIInterface();

    /* Sets the current tick */
    virtual void setTick(unsigned int);
    /* Stops a note playing on a MIDI channel using requested velocity */
    virtual void noteOff(unsigned char, unsigned char, unsigned char);
    /* Plays a note on a MIDI channel using requested velocity */
    virtual void noteOn(unsigned char, unsigned char, unsigned char);
    /* Sets the aftertouch pressure of a note playing on a MIDI channel */
    virtual void aftertouch(unsigned char, unsigned char, unsigned char);
    /* Sets the MIDI controller value of a MIDI channel */
    virtual void controller(unsigned char, unsigned char, unsigned char);
    /* Send a program change on a MIDI channel */
    virtual void programChange(unsigned char, unsigned char);
    /* Sets the channel pressure of a MIDI channel */
    virtual void channelPressure(unsigned char, unsigned char);
    /* Sets the pitch wheel value of a MIDI channel */
    virtual void pitchWheel(unsigned char, unsigned short);
    /* Sends a MIDI message */
    virtual void writeRaw(const QByteArray &data);
    /* Send a clock message */
    virtual void clock();
    /* Set the tempo (used when exporting) */
    virtual void tempo(unsigned int);
};

#endif // MIDIINTERFACE_H
