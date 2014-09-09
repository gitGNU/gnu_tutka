/*
 * block.h
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

#ifndef BLOCK_H_
#define BLOCK_H_

#include <QObject>
#include <QString>

class QDomElement;
class QDomDocument;

class Block : public QObject {
    Q_OBJECT

public:
    // Allocates a block
    Block(unsigned int tracks = 16, unsigned int length = 64, unsigned int commandPages = 1, QObject *parent = NULL);
    virtual ~Block();

    // Returns the name of the block
    QString name() const;

    // Sets the name of the block
    void setName(const QString &name_);

    // Returns the number of tracks in the block
    unsigned int tracks() const;

    // Sets the number of tracks in a block
    void setTracks(unsigned int tracks);

    // Returns the length of the block
    unsigned int length() const;

    // Sets the length of a block
    void setLength(unsigned int length);

    // Returns the number of command pages in the block
    unsigned int commandPages() const;

    // Sets the number of command pages in a block
    void setCommandPages(unsigned int commandPages);

    // Gets a note from a block
    unsigned char note(unsigned int line, unsigned int track);

    // Sets a note in a block
    void setNote(unsigned int line, unsigned int track, unsigned char octave, unsigned char note, unsigned char instrument);

    // Sets a note in a block
    void setNoteFull(unsigned int line, unsigned int track, unsigned char note, unsigned char instrument);

    // Gets an instrument from a block
    unsigned char instrument(unsigned int line, unsigned int track);

    // Sets an instrument in a block
    void setInstrument(unsigned int line, unsigned int track, unsigned char instrument);

    // Gets a command from a block
    unsigned char command(unsigned int line, unsigned int track, unsigned int commandPage);

    // Gets a command value from a block
    unsigned char commandValue(unsigned int line, unsigned int track, unsigned int commandPage);

    // Sets a part of a command in a block
    void setCommand(unsigned int line, unsigned int track, unsigned int commandPage, unsigned char slot, unsigned char data);

    // Sets a command in a block
    void setCommandFull(unsigned int line, unsigned int track, unsigned int commandPage, unsigned char command, unsigned char data);

    // Copies a part of a block to a new block
    Block *copy(int startTrack, int startLine, int endTrack, int endLine);

    // Pastes a block to another block in the given position
    void paste(Block *from, int track, int line);

    // Clears a part of a block
    void clear(int startTrack, int startLine, int endTrack, int endLine);

    // Transposes a block or a part of it
    void transpose(int instrument, int halfNotes, int startTrack, int startLine, int endTrack, int endLine);

    // Expands/shrinks a block or a part of it
    void expandShrink(int factor, int startTrack, int startLine, int endTrack, int endLine);

    // Changes or swaps an instrument to another
    void changeInstrument(int from, int to, bool swap, int startTrack, int startLine, int endTrack, int endLine);

    // Inserts a line to specific tracks in the block
    void insertLine(int line, int track = -1);

    // Deletes a line from specific tracks in the block
    void deleteLine(int line, int track = -1);

    // Inserts a track in the block
    void insertTrack(int track);

    // Deletes a track from the block
    void deleteTrack(int track);

    // Splits the rest of the block into a new block
    Block *split(int line);

    // Parses a block element in an XML file
    static Block *parse(QDomElement element);

    // Saves a block to an XML document
    void save(int number, QDomElement &parentElement, QDomDocument &document);

signals:
    // Emitted when a part of the block changes
    void areaChanged(int startTrack, int startLine, int endTrack, int endLine);

    // Emitted when the number of tracks changes
    void tracksChanged(int tracks);

    // Emitted when the length changes
    void lengthChanged(int length);

    // Emitted when the number of command pages changes
    void commandPagesChanged(int commandPages);

    // Emitted when the name of the block changes
    void nameChanged(QString name);

private:
    // Makes sure the given area is inside the block
    void checkBounds(int &startTrack, int &startLine, int &endTrack, int &endLine);

    // Name
    QString name_;
    // Number of tracks
    unsigned int tracks_;
    // Number of lines
    unsigned int length_;
    // Notation data
    unsigned char *notes_;
    // Number of command pages
    unsigned int commandPages_;
    // Command block array
    unsigned char *commands_;
};

#endif // BLOCK_H_
