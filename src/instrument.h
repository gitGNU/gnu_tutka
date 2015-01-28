/*
 * instrument.h
 *
 * Copyright 2002-2014 Vesa Halttunen
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

#ifndef INSTRUMENT_H_
#define INSTRUMENT_H_

#include <QObject>
#include <QString>

class Block;
class QDomElement;
class QDomDocument;

class Instrument : public QObject {
    Q_OBJECT

public:
    Instrument(const QString &name = "Unnamed", unsigned int midiInterface = 0, QObject *parent = NULL);
    virtual ~Instrument();

    // Returns the name of the instrument
    QString name() const;

    // Returns the MIDI interface number of the instrument
    unsigned int midiInterface() const;

    // Returns the MIDI interface name of the instrument
    QString midiInterfaceName() const;

    // Returns the MIDI preset of the instrument
    unsigned short midiPreset() const;

    // Returns the MIDI channel of the instrument
    unsigned char midiChannel() const;

    // Returns the default velocity of the instrument
    unsigned char defaultVelocity() const;

    // Returns the transposition of the instrument
    char transpose() const;

    // Returns the hold time of the instrument
    unsigned char hold() const;

    // Returns the arpeggio block of the instrument
    Block *arpeggio() const;

    // Returns the arpeggio base note of the instrument
    unsigned char basenote() const;

    // Parses an instrument element in an XML file
    static Instrument *parse(QDomElement element);

    // Saves an instrument to an XML document
    void save(int number, QDomElement &parentElement, QDomDocument &document);

public slots:
    // Sets the name of the instrument
    void setName(const QString &name);

    // Sets the MIDI channel of the instrument
    void setMidiChannel(int midiChannel);

    // Sets the default velocity of the instrument
    void setDefaultVelocity(int defaultVelocity);

    // Sets the transposition of the instrument
    void setTranspose(int transpose);

    // Sets the hold time of the instrument
    void setHold(int hold);

    // Sets the arpeggio block of the instrument
    void setArpeggio(Block *arpeggio);

    // Sets the midi interface of the instrument
    void setMidiInterface(int interface);

    // Sets the midi interface name of the instrument
    void setMidiInterfaceName(const QString &midiInterfaceName);

signals:
    // Emitted when the name has changed
    void nameChanged(QString name);

private:
    // Name
    QString name_;
    // MIDI interface
    unsigned int midiInterface_;
    // MIDI interface name (for remapping)
    QString midiInterfaceName_;
    // MIDI preset
    unsigned short midiPreset_;
    // MIDI channel
    unsigned char midiChannel_;
    // Default velocity
    unsigned char defaultVelocity_;
    // Transpose
    char transpose_;
    // Holding time in ticks
    unsigned char hold_;
    // Arpeggio block
    Block *arpeggio_;
    // Arpeggio base note
    unsigned char basenote_;
};

#endif // INSTRUMENT_H_
