/*
 * song.h
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

#ifndef SONG_H_
#define SONG_H_

#include <QObject>
#include <QMutex>
#include "playseq.h"
#include "block.h"
#include "instrument.h"
#include "message.h"

class QDomElement;
class Track;

class Song : public QObject {
    Q_OBJECT

public:
    // Loads a song from an XML file or creates a new song
    Song(const QString &path = QString(), QObject *parent = NULL);

    // Frees a song structure and its contents
    virtual ~Song();

    // Returns the name of the song
    QString name() const;

    // Returns the tempo of the song
    unsigned int tempo() const;

    // Returns the number of ticks per line in the song
    unsigned int ticksPerLine() const;

    // Returns whether MIDI sync should be sent
    bool sendSync() const;

    // Returns the master volume
    unsigned int masterVolume() const;

    // Sets the master volume
    void setMasterVolume(unsigned int masterVolume);

    // Returns the path the song was last stored to
    QString path() const;

    // Returns the number of blocks
    unsigned int blocks() const;

    // Returns the number of playing sequences
    unsigned int playseqs() const;

    // Returns the number of sections
    unsigned int sections() const;

    // Returns the number of instruments
    unsigned int instruments() const;

    // Returns the number of messages
    unsigned int messages() const;

    // Returns the maximum number of tracks
    unsigned int maxTracks() const;

    // Returns the given block
    Block *block(unsigned int number) const;

    // Returns the given track
    Track *track(unsigned int number) const;

    // Returns the given playing sequence
    Playseq *playseq(unsigned int number) const;

    // Returns the given section
    unsigned int section(unsigned int pos) const;

    // Returns the given instrument
    Instrument *instrument(unsigned int number) const;

    // Returns the given message
    Message *message(unsigned int number) const;

    // Inserts a new block in the block array in the given position
    void insertBlock(unsigned int pos, unsigned int current);

    // Deletes a block from the given position of the block array
    void deleteBlock(unsigned int pos);

    // Splits the block in the given position into two from the given line
    void splitBlock(unsigned int pos, unsigned int line);

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

    // Make sure the instrument exists; add instruments if necessary
    void checkInstrument(int instrument);

    // Transposes all blocks in a song
    void transpose(int instrument, int halfNotes);

    // Expands/shrinks all blocks in a song
    void expandShrink(int factor);

    // Changes or swaps an instrument with another in all blocks of a song
    void changeInstrument(int from, int to, bool swap);

    // Saves a song to an XML file
    void save(const QString &path);

    // Locks the song
    void lock();

    // Unlocks the song
    void unlock();

public slots:
    // Sets the number of ticks per line for the song
    void setTPL(int ticksPerLine);

    // Sets the tempo of the song
    void setTempo(int tempo);

    // Sets the name of the song
    void setName(const QString &name);

    // Sets whether to send sync
    void setSendSync(bool sendSync);

private slots:
    // If the maximum number of tracks has changed recreate the track volumes
    void checkMaxTracks();

signals:
    // Emitted when the number of blocks has changed
    void blocksChanged(int blocks);

    // Emitted when the number of playing sequences has changed
    void playseqsChanged(int playseqs);

    // Emitted when the number of sections has changed
    void sectionsChanged(unsigned int sections);

    // Emitted when the number of messages has changed
    void messagesChanged(unsigned int messages);

    // Emitted when the maximum number of tracks has changed
    void maxTracksChanged(unsigned int maxTracks);

    // Emitted when a name of a playing sequence has changed
    void playseqNameChanged();

    // Emitted when a name of a block has changed
    void blockNameChanged();

    // Emitted when a track is muted or soloed
    void trackMutedOrSoloed();

    // Emitted when a name of a track has changed
    void trackNameChanged();

    // Emitted when the length of a block has changed
    void blockLengthChanged();

private:
    // Initializes an empty song
    void init();

    // Parses a song element in an XML file
    bool parse(QDomElement element);

    // Creates a new track and associates it with this song
    void addTrack(const QString &name = QString());

    // Connects signals related to a block
    void connectBlockSignals(Block *block);

    // Name of the song
    QString name_;
    // Tempo, ticks per line
    unsigned int tempo_, ticksPerLine_;
    // Section array
    QList<unsigned int> sections_;
    // Playing sequence array
    QList<Playseq *> playseqs_;
    // Block array
    QList<Block *> blocks_;
    // Instrument array
    QList<Instrument *> instruments_;
    // Track volume array
    QList<Track *> tracks;
    // Master volume
    unsigned int masterVolume_;
    // System Exclusive messages
    QList<Message *> messages_;
    // Whether to send MIDI sync or not
    bool sendSync_;
    // Path the song was last stored to
    QString path_;
    // Mutex for locking the song
    QMutex mutex;
};

#endif // SONG_H_
