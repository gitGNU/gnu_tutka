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

#include <QObject>
#include "playseq.h"
#include "block.h"
#include "instrument.h"
#include "message.h"

class QDomElement;

class Track : public QObject {
    Q_OBJECT

public:
    Track(const QString &name = QString(), QObject *parent = NULL);
    virtual ~Track();

    QString name() const;
    int volume() const;
    bool isMuted() const;
    bool isSolo() const;

public slots:
    void setName(const QString &name);
    void setVolume(int volume);
    void setMute(bool mute);
    void setSolo(bool solo);

private:
    // Name of the track
    QString name_;
    // Track volume
    int volume_;
    // Mute toggle
    bool mute;
    // Solo toggle
    bool solo;
};

class Song : public QObject {
    Q_OBJECT

public:
    // Loads a song from an XML file or creates a new song
    Song(const QString &path = QString(), QObject *parent = NULL);
    // Frees a song structure and its contents
    virtual ~Song();

    // Inserts a new block in the block array in the given position
    void insertBlock(unsigned int pos, unsigned int current);
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
    // Make sure the instrument exists; add instruments if necessary
    void checkInstrument(int instrument, unsigned short defaultMIDIInterface = 0);
    // Saves a song to an XML file
    void save(const QString &path);
    // Transposes all blocks in a song
    void transpose(int instrument, int halfNotes);
    // Expands/shrinks all blocks in a song
    void expandShrink(int factor);
    // Changes or swaps an instrument with another in all blocks of a song
    void changeInstrument(int from, int to, bool swap);

    Block *block(unsigned int number) const;
    Track *track(unsigned int number) const;
    Playseq *playseq(unsigned int number) const;
    Instrument *instrument(unsigned int number) const;
    Message *message(unsigned int number) const;
    unsigned int maxTracks() const;
    unsigned int blocks() const;
    unsigned int playseqs() const;
    unsigned int sections() const;
    unsigned int instruments() const;
    unsigned int messages() const;
    unsigned int section(unsigned int pos) const;
    unsigned int masterVolume() const;
    QString name() const;
    unsigned int tempo() const;
    unsigned int ticksPerLine() const;
    bool sendSync() const;
    QString path() const;

public slots:
    // Sets the number of ticks per line for a song
    void setTPL(int ticksPerLine);
    // Sets the tempo of a song
    void setTempo(int tempo);
    void setName(const QString &name);
    void setSendSync(bool sendSync);

private slots:
    // If the maximum number of tracks has changed recreate the track volumes
    void checkMaxTracks();

signals:
    void blocksChanged(unsigned int blocks);
    void playseqsChanged(unsigned int playseqs);
    void sectionsChanged(unsigned int sections);
    void messagesChanged(unsigned int messages);
    void maxTracksChanged(unsigned int maxTracks);

private:
    // Initializes an empty song
    void init();
    // Parses a song element in an XML file
    bool parse(QDomElement element);

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
};

#endif // SONG_H_
