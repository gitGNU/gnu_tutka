/*
 * song.h
 *
 * Copyright 2002-2011 vesuri
 *
 * This file is part of Tutka2.
 *
 * Tutka2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka2; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SONG_H_
#define SONG_H_

#include "playseq.h"
#include "block.h"
#include "instrument.h"
#include "message.h"

class QDomElement;

class Track {
public:
    Track(const QString &name = QString());
    virtual ~Track();

    // Name of the track
    QString name;
    // Track volume
    unsigned int volume;
    // Mute toggle
    bool mute;
    // Solo toggle
    bool solo;
};

class Song {
public:
    // Allocates a new song structure and initializes it to default values
    Song(const QString &name = "Untitled");
    // Frees a song structure and its contents
    virtual ~Song();

    // Inserts a new block in the block array in the given position
    void insertBlock(unsigned int pos, int current);
    // Deletes a block from the given position of the block array
    void deleteBlock(unsigned int pos);
    // Inserts a new playseq in the playseq array in the given position
    void insertPlayseq(unsigned int pos);
    // Deletes a playseq from the given position of the playseq array
    void deletePlayseq(unsigned int pos);
    // Inserts a new section in the section array in the given position
    void insertSection(unsigned int pos);
    // Deletes a section from the given position of the section array
    void deleteSection(unsigned int pos);
    // Inserts a new Message in the Message array in the given position
    void insertMessage(unsigned int pos);
    // Deletes a Message from the given position of the Message array
    void deleteMessage(unsigned int pos);
    // Sets a section in the given position to point somewhere
    void setSection(unsigned int pos, unsigned int playseq);
    // Sets the number of ticks per line for a song
    void setTPL(unsigned int ticksPerLine);
    // Sets the tempo of a song
    void setTempo(unsigned int tempo);
    // If the maximum number of tracks has changed recreate the track volumes
    bool checkMaxTracks();
    // Make sure the instrument exists; add instruments if necessary
    void checkInstrument(int instrument, unsigned short defaultMIDIInterface);
    // Loads a song from an XML file
    static Song *load(const QString &path);
    // Saves a song to an XML file
    //void save(const QString &path);
    // Transposes all blocks in a song
    void transpose(int instrument, int halfNotes);
    // Expands/shrinks all blocks in a song
    void expandShrink(int factor);
    // Changes or swaps an instrument with another in all blocks of a song
    void changeInstrument(int from, int to, bool swap);

    Block *block(int number);
    Track *track(int number);

private:
    // Parses a song element in an XML file
    static Song *parse(QDomElement element);

    // Name of the song
    QString name;
    // Tempo, ticks per line
    unsigned int tempo, ticksPerLine;
    // Section array
    QList<unsigned int> sections;
    // Playing sequence array
    QList<Playseq> playseqs;
    // Block array
    QList<Block> blocks;
    // Instrument array
    QList<Instrument> instruments;
    // Track volume array
    QList<Track> tracks;
    // Master volume
    unsigned int masterVolume;
    // System Exclusive messages
    QList<Message> messages;
    // Whether to send MIDI sync or not
    bool sendSync;
};

#endif // SONG_H_
