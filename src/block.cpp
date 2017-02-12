/*
 * block.cpp
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

#include <cstddef>
#include <cstdlib>
#include <QDomElement>
#include "block.h"

Block::Block(unsigned int tracks, unsigned int length, unsigned int commandPages, QObject *parent) :
    QObject(parent),
    tracks_(tracks),
    length_(length),
    notes_(NULL),
    commandPages_(commandPages),
    commands_(NULL)
{
    notes_ = (unsigned char *)calloc(2 * tracks * length, sizeof(unsigned char));
    commands_ = (unsigned char *)calloc(commandPages * 2 * tracks * length, sizeof(unsigned char));
}

Block::~Block()
{
    free(notes_);
    free(commands_);
}

QString Block::name() const
{
    return name_;
}

void Block::setName(const QString &name)
{
    name_ = name;

    emit nameChanged(name_);
}

unsigned int Block::tracks() const
{
    return tracks_;
}

void Block::setTracks(unsigned int tracks)
{
    // Allocate new arrays
    unsigned char *notes = (unsigned char *)calloc(2 * tracks * length_, sizeof(unsigned char));
    unsigned char *commands = (unsigned char *)calloc(commandPages_ * 2 * tracks * length_, sizeof(unsigned char));
    unsigned int oldTracks = tracks_;

    // How many tracks from the old block to use
    unsigned int existingTracks = tracks < oldTracks ? tracks : oldTracks;

    // Copy the notes and the commands of the block to a new data array
    for (unsigned int line = 0; line < length_; line++) {
        for (unsigned int track = 0; track < existingTracks; track++) {
            notes[(line * tracks + track) * 2] = notes_[(line * oldTracks + track) * 2];
            notes[(line * tracks + track) * 2 + 1] = notes_[(line * oldTracks + track) * 2 + 1];
            for (unsigned int commandPage = 0; commandPage < commandPages_; commandPage++) {
                commands[commandPage * 2 * tracks * length_ + (line * tracks + track) * 2] = commands_[commandPage * 2 * oldTracks * length_ + (line * oldTracks + track) * 2];
                commands[commandPage * 2 * tracks * length_ + (line * tracks + track) * 2 + 1] = commands_[commandPage * 2 * oldTracks * length_ + (line * oldTracks + track) * 2 + 1];
            }
        }
    }

    // Free old arrays
    free(notes_);
    free(commands_);

    // Use new arrays
    notes_ = notes;
    commands_ = commands;
    tracks_ = tracks;

    emit tracksChanged(tracks_);
    emit areaChanged(tracks > oldTracks ? oldTracks : tracks, 0, (tracks > oldTracks ? tracks : oldTracks) - 1, length_ - 1);
}

unsigned int Block::length() const
{
    return length_;
}

void Block::setLength(unsigned int length)
{
    // Allocate new arrays
    unsigned char *notes = (unsigned char *)calloc(2 * tracks_ * length, sizeof(unsigned char));
    unsigned char *commands = (unsigned char *)calloc(commandPages_ * 2 * tracks_ * length, sizeof(unsigned char));
    unsigned int oldLength = length_;

    // How many lines from the old block to use
    unsigned int existingLength = length < oldLength ? length : oldLength;

    // Copy the notes and the commands of the block to a new data array
    for (unsigned int line = 0; line < existingLength; line++) {
        for (unsigned int track = 0; track < tracks_; track++) {
            notes[(line * tracks_ + track) * 2] = notes_[(line * tracks_ + track) * 2];
            notes[(line * tracks_ + track) * 2 + 1] = notes_[(line * tracks_ + track) * 2 + 1];
            for (unsigned int commandPage = 0; commandPage < commandPages_; commandPage++) {
                commands[commandPage * 2 * tracks_ * length + (line * tracks_ + track) * 2] = commands_[commandPage * 2 * tracks_ * oldLength + (line * tracks_ + track) * 2];
                commands[commandPage * 2 * tracks_ * length + (line * tracks_ + track) * 2 + 1] = commands_[commandPage * 2 * tracks_ * oldLength + (line * tracks_ + track) * 2 + 1];
            }
        }
    }

    // Free old arrays
    free(notes_);
    free(commands_);

    // Use new arrays
    notes_ = notes;
    commands_ = commands;
    length_ = length;

    emit lengthChanged(length_);
    emit areaChanged(0, length > oldLength ? oldLength : length, tracks_ - 1, (length > oldLength ? length : oldLength) - 1);
}

unsigned int Block::commandPages() const
{
    return commandPages_;
}

void Block::setCommandPages(unsigned int commandPages)
{
    // Allocate new command page array
    unsigned char *commands = (unsigned char *)calloc(commandPages * 2 * tracks_ * length_, sizeof(unsigned char));

    // How many command pages from the old block to use
    unsigned int existingCommandPages = commandPages < commandPages_ ? commandPages : commandPages_;

    // Copy the command pages to a new data array
    memcpy(commands, commands_, existingCommandPages * 2 * tracks_ * length_);

    // Free old array
    free(commands_);

    // Use new array
    commands_ = commands;
    commandPages_ = commandPages;

    emit commandPagesChanged(commandPages_);
    emit areaChanged(0, 0, tracks_ - 1, length_ - 1);
}

unsigned char Block::note(unsigned int line, unsigned int track)
{
    return notes_[2 * (tracks_ * line + track)];
}

void Block::setNote(unsigned int line, unsigned int track, unsigned char octave, unsigned char note, unsigned char instrument)
{
    if (note != 0) {
        notes_[2 * (tracks_ * line + track)] = octave * 12 + note;
        notes_[2 * (tracks_ * line + track) + 1] = instrument;
    } else {
        notes_[2 * (tracks_ * line + track)] = 0;
        notes_[2 * (tracks_ * line + track) + 1] = 0;
    }

    emit areaChanged(track, line, track, line);
}

void Block::setNoteFull(unsigned int line, unsigned int track, unsigned char note, unsigned char instrument)
{
    notes_[2 * (tracks_ * line + track)] = note;
    notes_[2 * (tracks_ * line + track) + 1] = instrument;

    emit areaChanged(track, line, track, line);
}

unsigned char Block::instrument(unsigned int line, unsigned int track)
{
    return notes_[2 * (tracks_ * line + track) + 1];
}

void Block::setInstrument(unsigned int line, unsigned int track, unsigned char instrument)
{
    notes_[2 * (tracks_ * line + track) + 1] = instrument;

    emit areaChanged(track, line, track, line);
}

unsigned char Block::command(unsigned int line, unsigned int track, unsigned int commandPage)
{
    return commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track)];
}

unsigned char Block::commandValue(unsigned int line, unsigned int track, unsigned int commandPage)
{
    return commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + 1];
}

void Block::setCommand(unsigned int line, unsigned int track, unsigned int commandPage, unsigned char slot, unsigned char data)
{
    if ((slot & 1) != 0) {
        commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + slot / 2] &= 0xf0;
        commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + slot / 2] |= data;
    } else {
        commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + slot / 2] &= 0x0f;
        commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + slot / 2] |= (data << 4);
    }

    emit areaChanged(track, line, track, line);
}

void Block::setCommandFull(unsigned int line, unsigned int track, unsigned int commandPage, unsigned char command, unsigned char data)
{
    commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track)] = command;
    commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + 1] = data;

    emit areaChanged(track, line, track, line);
}

Block *Block::copy(int startTrack, int startLine, int endTrack, int endLine)
{
    checkBounds(startTrack, startLine, endTrack, endLine);

    // Allocate new block
    Block *newBlock = new Block(endTrack - startTrack + 1, endLine - startLine + 1, commandPages_);
    unsigned int oldLength = length_;
    unsigned int newLength = newBlock->length_;
    unsigned int oldTracks = tracks_;
    unsigned int newTracks = newBlock->tracks_;

    // Copy the given part of the block to a new block
    for (int line = startLine; line <= endLine; line++) {
        for (int track = startTrack; track <= endTrack; track++) {
            newBlock->notes_[((line - startLine) * newTracks + (track - startTrack)) * 2] = notes_[(line * oldTracks + track) * 2];
            newBlock->notes_[((line - startLine) * newTracks + (track - startTrack)) * 2 + 1] = notes_[(line * oldTracks + track) * 2 + 1];
            for (unsigned int commandPage = 0; commandPage < commandPages_; commandPage++) {
                newBlock->commands_[commandPage * 2 * newTracks * newLength + ((line - startLine) * newTracks + (track - startTrack)) * 2] = commands_[commandPage * 2 * oldTracks * oldLength + (line * oldTracks + track) * 2];
                newBlock->commands_[commandPage * 2 * newTracks * newLength + ((line - startLine) * newTracks + (track - startTrack)) * 2 + 1] = commands_[commandPage * 2 * oldTracks * oldLength + (line * oldTracks + track) * 2 + 1];
            }
        }
    }

    return newBlock;
}

void Block::paste(Block *from, int track, int line)
{
    int toLength = length_;
    int toTracks = tracks_;
    int toCommandPages = commandPages_;
    int fromLength = from->length_;
    int fromTracks = from->tracks_;
    int fromCommandPages = from->commandPages_;
    int copyLength = fromLength;
    int copyTracks = fromTracks;
    int copyCommandPages = fromCommandPages;

    if (line + fromLength > toLength) {
        copyLength = toLength - line;
    }
    if (track + fromTracks > toTracks) {
        copyTracks = toTracks - track;
    }
    if (copyCommandPages > toCommandPages) {
        copyCommandPages = toCommandPages;
    }

    // Copy the from block to the destination block; make sure it fits
    for (int l = 0; l < copyLength; l++) {
        for (int t = 0; t < copyTracks; t++) {
            notes_[((line + l) * toTracks + (track + t)) * 2] = from->notes_[(l * fromTracks + t) * 2];
            notes_[((line + l) * toTracks + (track + t)) * 2 + 1] = from->notes_[(l * fromTracks + t) * 2 + 1];
            for (int ep = 0; ep < copyCommandPages; ep++) {
                commands_[ep * 2 * toTracks * toLength + ((line + l) * toTracks + (track + t)) * 2] = from->commands_[ep * 2 * fromTracks * fromLength + (l * fromTracks + t) * 2];
                commands_[ep * 2 * toTracks * toLength + ((line + l) * toTracks + (track + t)) * 2 + 1] = from->commands_[ep * 2 * fromTracks * fromLength + (l * fromTracks + t) * 2 + 1];
            }
        }
    }

    emit areaChanged(track, line, track + copyTracks - 1, line + copyLength - 1);
}

void Block::clear(int startTrack, int startLine, int endTrack, int endLine)
{
    checkBounds(startTrack, startLine, endTrack, endLine);

    for (int line = startLine; line <= endLine; line++) {
        for (int track = startTrack; track <= endTrack; track++) {
            notes_[(line * tracks_ + track) * 2] = 0;
            notes_[(line * tracks_ + track) * 2 + 1] = 0;
            for (unsigned int commandPage = 0; commandPage < commandPages_; commandPage++) {
                commands_[commandPage * 2 * tracks_ * length_ + (line * tracks_ + track) * 2] = 0;
                commands_[commandPage * 2 * tracks_ * length_ + (line * tracks_ + track) * 2 + 1] = 0;
            }
        }
    }

    emit areaChanged(startTrack, startLine, endTrack, endLine);
}

void Block::transpose(int instrument, int halfNotes, int startTrack, int startLine, int endTrack, int endLine)
{
    checkBounds(startTrack, startLine, endTrack, endLine);

    // Either check for correct instrument or transpose all instruments
    if (instrument < 0) {
        for (int line = startLine; line <= endLine; line++) {
            for (int track = startTrack; track <= endTrack; track++) {
                if (notes_[(line * tracks_ + track) * 2] > 0) {
                    if (notes_[(line * tracks_ + track) * 2] + halfNotes < 1) {
                        notes_[(line * tracks_ + track) * 2] = 1;
                    } else if (notes_[(line * tracks_ + track) * 2] + halfNotes > 127) {
                        notes_[(line * tracks_ + track) * 2] = 127;
                    } else {
                        notes_[(line * tracks_ + track) * 2] += halfNotes;
                    }
                }
            }
        }
    } else {
        for (int line = startLine; line <= endLine; line++) {
            for (int track = startTrack; track <= endTrack; track++) {
                if (notes_[(line * tracks_ + track) * 2] > 0 && notes_[(line * tracks_ + track) * 2 + 1] == instrument + 1) {
                    if (notes_[(line * tracks_ + track) * 2] + halfNotes < 0) {
                        notes_[(line * tracks_ + track) * 2] = 1;
                    } else if (notes_[(line * tracks_ + track) * 2] + halfNotes > 127) {
                        notes_[(line * tracks_ + track) * 2] = 127;
                    } else {
                        notes_[(line * tracks_ + track) * 2] += halfNotes;
                    }
                }
            }
        }
    }

    emit areaChanged(startTrack, startLine, endTrack, endLine);
}

void Block::expandShrink(int factor, int startTrack, int startLine, int endTrack, int endLine, bool changeBlockLength)
{
    if (factor > -2 && factor < 2) {
        return;
    }

    checkBounds(startTrack, startLine, endTrack, endLine);

    int lines = endLine - startLine + 1;

    if (factor < 0) {
        // Shrink
        if (lines >= -factor)
        {
            int newEndLine = startLine + (endLine + 1 - startLine) / -factor - 1;
            for (int line = startLine; line <= newEndLine; line++) {
                for (int track = startTrack; track <= endTrack; track++) {
                    notes_[(line * tracks_ + track) * 2] = notes_[((startLine + (line - startLine) * -factor) * tracks_ + track) * 2];
                    notes_[(line * tracks_ + track) * 2 + 1] = notes_[((startLine + (line - startLine) * -factor) * tracks_ + track) * 2 + 1];
                    for (unsigned int commandPage = 0; commandPage < commandPages_; commandPage++) {
                        commands_[commandPage * tracks_ * length_ * 2 + (line * tracks_ + track) * 2] = commands_[commandPage * tracks_ * length_ * 2 + ((startLine + (line - startLine) * -factor) * tracks_ + track) * 2];
                        commands_[commandPage * tracks_ * length_ * 2 + (line * tracks_ + track) * 2 + 1] = commands_[commandPage * tracks_ * length_ * 2 + ((startLine + (line - startLine) * -factor) * tracks_ + track) * 2 + 1];
                    }
                }
            }

            if (changeBlockLength) {
                Block *block = copy(startTrack, endLine + 1, endTrack, length_ - 1);
                setLength(length_ - lines + lines / -factor);
                paste(block, startTrack, newEndLine + 1);
                delete block;
            } else {
                clear(startTrack, newEndLine + 1, endTrack, endLine);
            }
        }
    } else {
        // Expand
        if (changeBlockLength) {
            Block *block = copy(startTrack, endLine + 1, endTrack, length_ - 1);
            setLength(length_ + lines * (factor - 1));
            endLine = startLine + lines * factor - 1;
            paste(block, startTrack, endLine + 1);
            delete block;
        }

        for (int line = endLine; line >= startLine; line--) {
            if ((line - startLine) % factor == 0) {
                for (int track = startTrack; track <= endTrack; track++) {
                    notes_[(line * tracks_ + track) * 2] = notes_[((startLine + (line - startLine) / factor) * tracks_ + track) * 2];
                    notes_[(line * tracks_ + track) * 2 + 1] = notes_[((startLine + (line - startLine) / factor) * tracks_ + track) * 2 + 1];
                    for (unsigned int commandPage = 0; commandPage < commandPages_; commandPage++) {
                        commands_[commandPage * tracks_ * length_ * 2 + (line * tracks_ + track) * 2] = commands_[commandPage * tracks_ * length_ * 2 + ((startLine + (line - startLine) / factor) * tracks_ + track) * 2];
                        commands_[commandPage * tracks_ * length_ * 2 + (line * tracks_ + track) * 2 + 1] = commands_[commandPage * tracks_ * length_ * 2 + ((startLine + (line - startLine) / factor) * tracks_ + track) * 2 + 1];
                    }
                }
            } else {
                for (int track = startTrack; track <= endTrack; track++) {
                    notes_[(line * tracks_ + track) * 2] = 0;
                    notes_[(line * tracks_ + track) * 2 + 1] = 0;
                    for (unsigned int commandPage = 0; commandPage < commandPages_; commandPage++) {
                        commands_[commandPage * tracks_ * length_ * 2 + (line * tracks_ + track) * 2] = 0;
                        commands_[commandPage * tracks_ * length_ * 2 + (line * tracks_ + track) * 2 + 1] = 0;
                    }
                }
            }
        }
    }

    emit areaChanged(startTrack, startLine, endTrack, endLine);
}

void Block::changeInstrument(int from, int to, bool swap, int startTrack, int startLine, int endTrack, int endLine)
{
    checkBounds(startTrack, startLine, endTrack, endLine);

    if (swap) {
        for (int line = endLine; line >= startLine; line--) {
            for (int track = startTrack; track <= endTrack; track++) {
                if (notes_[(line * tracks_ + track) * 2 + 1] == from) {
                    notes_[(line * tracks_ + track) * 2 + 1] = to;
                } else if (notes_[(line * tracks_ + track) * 2 + 1] == to) {
                    notes_[(line * tracks_ + track) * 2 + 1] = from;
                }
            }
        }
    } else {
        for (int line = endLine; line >= startLine; line--) {
            for (int track = startTrack; track <= endTrack; track++) {
                if (notes_[(line * tracks_ + track) * 2 + 1] == from) {
                    notes_[(line * tracks_ + track) * 2 + 1] = to;
                }
            }
        }
    }

    emit areaChanged(startTrack, startLine, endTrack, endLine);
}

void Block::insertLine(int line, int track)
{
    int startTrack = track >= 0 ? track : 0;
    int endTrack = track >= 0 ? track : (tracks_ - 1);

    // Move lines downwards
    Block *block = copy(startTrack, line, endTrack, length_ - 1);
    paste(block, startTrack, line + 1);
    delete block;

    // Clear the inserted line
    clear(startTrack, line, endTrack, line);

    emit areaChanged(startTrack, line, endTrack, length_ - 1);
}

void Block::deleteLine(int line, int track)
{
    int startTrack = track >= 0 ? track : 0;
    int endTrack = track >= 0 ? track : (tracks_ - 1);

    // Move lines upwards
    Block *block = copy(startTrack, line + 1, endTrack, length_ - 1);
    paste(block, startTrack, line);
    delete block;

    // Clear the last line
    clear(startTrack, length_ - 1, endTrack, length_ - 1);

    emit areaChanged(startTrack, line, endTrack, length_ - 1);
}

void Block::insertTrack(int track)
{
    // Add a new track
    setTracks(tracks_ + 1);

    if (track < tracks_ - 1) {
        // Move tracks to the right
        Block *block = copy(track, 0, tracks_ - 1, length_ - 1);
        paste(block, track + 1, 0);
        delete block;

        // Clear the inserted track
        clear(track, 0, track, length_ - 1);
    }
}

void Block::deleteTrack(int track)
{
    if (tracks_ > 1 && track < tracks_) {
        if (track < tracks_ - 1) {
            // Move tracks to the left
            Block *block = copy(track + 1, 0, tracks_ - 1, length_ - 1);
            paste(block, track, 0);
            delete block;
        }

        // Remove a track
        setTracks(tracks_ - 1);
    }
}

Block *Block::split(int line)
{
    if (line > 0) {
        Block *block = copy(0, line, tracks_ - 1, length_ - 1);
        setLength(line);
        return block;
    } else {
        return NULL;
    }
}

Block *Block::parse(QDomElement element)
{
    Block *block = NULL;

    if (element.tagName() == "block") {
        QDomAttr prop;
        int tracks = 4, length = 64, commandpages = 1;

        // Get block properties
        prop = element.attributeNode("tracks");
        if (!prop.isNull()) {
            tracks = prop.value().toInt();
        }

        prop = element.attributeNode("length");
        if (!prop.isNull()) {
            length = prop.value().toInt();
        }

        prop = element.attributeNode("commandpages");
        if (!prop.isNull()) {
            commandpages = prop.value().toInt();
        }

        // Allocate block
        block = new Block(tracks, length, commandpages);
        prop = element.attributeNode("name");
        if (!prop.isNull()) {
            block->name_ = prop.value();
        }

        // Get block contents
        QDomElement cur = element.firstChild().toElement();
        while(!cur.isNull()) {
            if (cur.tagName() == "note") {
                int line = 0, track = 0, note = 0, instrument = 0;

                // Get note properties
                prop = cur.attributeNode("line");
                if (!prop.isNull()) {
                    line = prop.value().toInt();
                }

                prop = cur.attributeNode("track");
                if (!prop.isNull()) {
                    track = prop.value().toInt();
                }

                prop = cur.attributeNode("instrument");
                if (!prop.isNull()) {
                    instrument = prop.value().toInt();
                }

                // Get the note
                note = cur.text().toInt();

                // Set the note
                block->setNote(line, track, 0, note, instrument);
            } else if (cur.tagName() == "command") {
                int line = 0, track = 0, commandpage = 0, command = 0, value = 0;

                // Get command properties
                prop = cur.attributeNode("line");
                if (!prop.isNull()) {
                    line = prop.value().toInt();
                }

                prop = cur.attributeNode("track");
                if (!prop.isNull()) {
                    track = prop.value().toInt();
                }

                prop = cur.attributeNode("commandpage");
                if (!prop.isNull()) {
                    commandpage = prop.value().toInt();
                }

                prop = cur.attributeNode("value");
                if (!prop.isNull()) {
                    value = prop.value().toInt();
                }

                // Get the command
                command = cur.text().toInt();

                // Set the command
                block->setCommandFull(line, track, commandpage, command, value);
            }
            cur = cur.nextSibling().toElement();
        }
    } else if (element.nodeType() != QDomNode::CommentNode) {
        qWarning("XML error: expected block, got %s\n", element.tagName().toUtf8().constData());
    }

    return block;
}

void Block::save(int number, QDomElement &parentElement, QDomDocument &document)
{
    // Set block properties
    QDomElement blockElement = document.createElement("block");
    parentElement.appendChild(blockElement);
    blockElement.setAttribute("number", number);
    blockElement.setAttribute("name", name_);
    blockElement.setAttribute("tracks", tracks_);
    blockElement.setAttribute("length", length_);
    blockElement.setAttribute("commandpages", commandPages_);
    blockElement.appendChild(document.createTextNode("\n"));

    // Notation data
    for (int track = 0; track < tracks_; track++) {
        for (int line = 0; line < length_; line++) {
            if (notes_[2 * (tracks_ * line + track)] != 0 || notes_[2 * (tracks_ * line + track) + 1] != 0) {
                QDomElement noteElement = document.createElement("note");
                noteElement.appendChild(document.createTextNode(QString("%1").arg(notes_[2 * (tracks_ * line + track)])));
                noteElement.setAttribute("line", line);
                noteElement.setAttribute("track", track);
                noteElement.setAttribute("instrument", notes_[2 * (tracks_ * line + track) + 1]);
                blockElement.appendChild(noteElement);
                blockElement.appendChild(document.createTextNode("\n"));
            }
        }
    }

    // Command data
    for (int track = 0; track < tracks_; track++) {
        for (int line = 0; line < length_; line++) {
            for (int commandPage = 0; commandPage < commandPages_; commandPage++) {
                if (commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track)] != 0 || commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + 1] != 0) {
                    QDomElement commandElement = document.createElement("command");
                    commandElement.appendChild(document.createTextNode(QString("%1").arg(commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track)])));
                    commandElement.setAttribute("line", line);
                    commandElement.setAttribute("track", track);
                    commandElement.setAttribute("commandpage", commandPage);
                    commandElement.setAttribute("value", commands_[commandPage * 2 * tracks_ * length_ + 2 * (tracks_ * line + track) + 1]);
                    blockElement.appendChild(commandElement);
                    blockElement.appendChild(document.createTextNode("\n"));
                }
            }
        }
    }

    parentElement.appendChild(document.createTextNode("\n"));
}

void Block::checkBounds(int &startTrack, int &startLine, int &endTrack, int &endLine)
{
    if (startTrack < 0) {
        startTrack = 0;
    }
    if (startLine < 0) {
        startLine = 0;
    }
    if (startTrack >= tracks_) {
        startTrack = tracks_ - 1;
    }
    if (startLine >= length_) {
        startLine = length_ - 1;
    }
    if (endTrack < 0) {
        endTrack = 0;
    }
    if (endLine < 0) {
        endLine = 0;
    }
    if (endTrack >= tracks_) {
        endTrack = tracks_ - 1;
    }
    if (endLine >= length_) {
        endLine = length_ - 1;
    }

    // Make sure start and end are the correct way around
    if (startTrack > endTrack) {
        int track = startTrack;
        startTrack = endTrack;
        endTrack = track;
    }
    if (startLine > endLine) {
        int line = startLine;
        startLine = endLine;
        endLine = line;
    }
}
