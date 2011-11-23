/*
 * song.cpp
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

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QDebug>
#include <cstdio>
#include "song.h"

Track::Track(const QString &name) :
    name_(name),
    volume_(127),
    mute(false),
    solo(false)
{
}

Track::~Track()
{
}

void Track::setName(const QString &name)
{
    name_ = name;
}

QString Track::name() const
{
    return name_;
}

void Track::setVolume(unsigned int volume)
{
    volume_ = volume;
}

unsigned int Track::volume() const
{
    return volume_;
}

void Track::setMute(bool mute)
{
    mute = mute;
}

bool Track::isMuted() const
{
    return mute;
}

void Track::setSolo(bool solo)
{
    solo = solo;
}

bool Track::isSolo() const
{
    return solo;
}

Song::Song(const QString &name) :
    name(name),
    tempo(130),
    ticksPerLine(6),
    masterVolume(127),
    sendSync(false)
{
    sections.append(0);
    playseqs.append(new Playseq);
    blocks.append(new Block(4, 64, 1));
    for (int i = 1; i <= 4; i++) {
        // Give a descriptive name for the new track
        tracks.append(new Track(QString("Track %1").arg(i)));
    }
    masterVolume = 127;
}

Song::~Song()
{
    foreach(Playseq *playseq, playseqs) {
        delete playseq;
    }
    foreach(Block *block, blocks) {
        delete block;
    }
    foreach(Instrument *instrument, instruments) {
        delete instrument;
    }
    foreach(Track *track, tracks) {
        delete track;
   }
    foreach(Message *message, messages) {
        delete message;
    }
}

void Song::insertBlock(unsigned int pos, int current)
{
    // Check block existence
    if (pos > blocks.count()) {
        pos = blocks.count();
    }

    // Insert a new block similar to the current block
    blocks.insert(pos, new Block(blocks[current]->tracks(), blocks[current]->length(), blocks[current]->commandPages()));

    // Update playing sequences
    for (int i = 0; i < playseqs.count(); i++) {
        for (unsigned int j = 0; j < playseqs[i]->length(); j++) {
            if (playseqs[i]->at(j) >= pos) {
                playseqs[i]->set(j, playseqs[i]->at(j) + 1);;
            }
        }
    }
}

void Song::deleteBlock(unsigned int pos)
{
    // Don't delete the last block
    if (blocks.count() > 1) {
        // Check block existence
        if (pos >= blocks.count()) {
            pos = blocks.count() - 1;
        }

        blocks.removeAt(pos);

        // Update playing sequences
        for (int i = 0; i < playseqs.count(); i++) {
            for (int j = 0; j < playseqs[i]->length(); j++) {
                if (playseqs[i]->at(j) >= pos && playseqs[i]->at(j) > 0) {
                    playseqs[i]->set(j, playseqs[i]->at(j) - 1);
                }
            }
        }
    }
}

// Inserts a new playseq in the playseq array in the given position
void Song::insertPlayseq(unsigned int pos)
{
    // Check playseq existence
    if (pos > playseqs.count()) {
        pos = playseqs.count();
    }

    // Insert a new playing sequence
    playseqs.insert(pos, new Playseq);

    // Update sections
    for (int i = 0; i < sections.count(); i++) {
        if (sections[i] >= pos) {
            sections[i]++;
        }
    }
}

void Song::deletePlayseq(unsigned int pos)
{
    // Don't delete the last playseq
    if (playseqs.count() > 1) {
        // Check playseq existence
        if (pos >= playseqs.count()) {
            pos = playseqs.count() - 1;
        }

        playseqs.removeAt(pos);

        // Update section lists
        for (int i = 0; i < sections.count(); i++) {
            if (sections[i] >= pos && sections[i] > 0) {
                sections[i]--;
            }
        }
    }
}

void Song::insertSection(unsigned int pos)
{
    // Check that the value is possible
    if (pos > sections.count()) {
        pos = sections.count();
    }

    sections.insert(pos, pos < sections.count() ? sections[pos] : sections[sections.count() - 1]);
}

void Song::deleteSection(unsigned int pos)
{
    // Don't delete the last section
    if (sections.count() > 1) {
        // Check section existence
        if (pos >= sections.count()) {
            pos = sections.count() - 1;
        }

        sections.removeAt(pos);
    }
}

void Song::insertMessage(unsigned int pos)
{
    // Check message existence
    if (pos > messages.count()) {
        pos = messages.count();
    }

    // Insert a new message
    messages.insert(pos, new Message());
}

void Song::deleteMessage(unsigned int pos)
{
    // Don't delete inexisting messages
    if (messages.count() > 0) {
        // Check message existence
        if (pos >= messages.count()) {
            pos = messages.count() - 1;
        }

        // Free the message in question
        messages.removeAt(pos);
    }
}

void Song::setSection(unsigned int pos, unsigned int playseq)
{
    if (pos < sections.count() && playseq < playseqs.count()) {
        sections[pos] = playseq;
    }
}

void Song::setTPL(unsigned int ticksPerLine)
{
    this->ticksPerLine = ticksPerLine;
}

void Song::setTempo(unsigned int tempo)
{
    this->tempo = tempo;
}

bool Song::checkMaxTracks()
{
    int max = 0;
    int oldMax = tracks.count();

    // Check the maximum number of tracks;
    for (int i = 0; i < blocks.count(); i++) {
        if (blocks[i]->tracks() > max) {
            max = blocks[i]->tracks();
        }
    }

    if (oldMax == max) {
        // Do nothing if the maximum number of tracks has not changed
        return false;
    } else if (oldMax < max) {
        for (int i = oldMax; i < max; i++) {
            // Give a descriptive name for the new track
            tracks.append(new Track(QString("Track %1").arg(i + 1)));
        }
    } else {
        // Tracks removed: free track datas
        while (tracks.count() > max) {
            delete tracks.takeLast();
        }
     }

    return true;
}

void Song::checkInstrument(int instrument, unsigned short defaultMIDIInterface)
{
    while (instrument >= instruments.count()) {
        instruments.append(new Instrument("Unnamed", defaultMIDIInterface));
    }
}

void Song::transpose(int instrument, int halfNotes)
{
    for (int i = 0; i < blocks.count(); i++) {
        blocks[i]->transpose(instrument, halfNotes, 0, 0, blocks[i]->tracks() - 1, blocks[i]->length() - 1);
    }
}

void Song::expandShrink(int factor)
{
    for (int i = 0; i < blocks.count(); i++) {
        blocks[i]->expandShrink(factor, 0, 0, blocks[i]->tracks() - 1, blocks[i]->length() - 1);
    }
}

void Song::changeInstrument(int from, int to, bool swap)
{
    for (int i = 0; i < blocks.count(); i++) {
        blocks[i]->changeInstrument(from, to, swap, 0, 0, blocks[i]->tracks() - 1, blocks[i]->length() - 1);
    }
}

Song *Song::load(const QString &path)
{
    QDomDocument doc;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return NULL;
    }
    if (!doc.setContent(&file)) {
        file.close();
        return NULL;
    }
    file.close();

    return parse(doc.documentElement());
}

Song *Song::parse(QDomElement element)
{
    Song *song = NULL;

    /*
    QDomNode node = element.firstChild();
    while(!node.isNull()) {
        QDomElement childElement = node.toElement();
        if(!childElement.isNull()) {
            qDebug() << qPrintable(childElement.tagName());
        }
        node = node.nextSibling();
    }
    */

    if (element.tagName() == "song") {
        QDomAttr prop;

        // Allocate song
        song = new Song;
        prop = element.attributeNode("name");
        if (!prop.isNull()) {
            song->name = prop.value();
        }

        prop = element.attributeNode("tempo");
        if (!prop.isNull()) {
            song->tempo = prop.value().toInt();
        }

        prop = element.attributeNode("ticksperline");
        if (!prop.isNull()) {
            song->ticksPerLine = prop.value().toInt();
        }

        prop = element.attributeNode("mastervolume");
        if (!prop.isNull()) {
            song->masterVolume = prop.value().toInt();
        }

        prop = element.attributeNode("sendsync");
        if (!prop.isNull()) {
            song->sendSync = (prop.value().toInt() == 1);
        }

        QDomElement cur = element.firstChild().toElement();
        while(!cur.isNull()) {
            if (cur.tagName() == "blocks") {
                // Parse and add all block elements
                QDomElement temp = cur.firstChild().toElement();

                while (!temp.isNull()) {
                    if (temp.isElement()) {
                        int number = -1;
                        Block *block = Block::parse(temp);

                        if (block != NULL) {
                            prop = temp.attributeNode("number");
                            if (!prop.isNull()) {
                                number = prop.value().toInt();
                            }

                            while (song->blocks.count() < number) {
                                song->blocks.append(new Block);
                            }
                            if (song->blocks.count() == number) {
                                song->blocks.append(block);
                            } else {
                                delete song->blocks.takeAt(number);
                                song->blocks.insert(number, block);
                            }
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            } else if (cur.tagName() == "sections") {
                // Parse and add all section elements
                QDomElement temp = cur.firstChild().toElement();

                while (!temp.isNull()) {
                    if (temp.isElement()) {
                        int number = -1;

                        // The section number is required
                        if (temp.tagName() == "section") {
                            prop = temp.attributeNode("number");
                            if (!prop.isNull()) {
                                number = prop.value().toInt();
                            }

                            // Get playing sequence
                            QDomElement temp2 = temp.firstChild().toElement();
                            if (!temp2.isNull()) {
                                while (song->sections.count() < number) {
                                    song->sections.append(0);
                                }
                                if (song->sections.count() == number) {
                                    song->sections.append(temp2.text().toInt());
                                } else {
                                    song->sections.replace(number, temp2.text().toInt());
                                }
                            }
                        } else if (temp.nodeType() != QDomNode::CommentNode) {
                            qWarning("XML error: expected section, got %s\n", temp.tagName().toUtf8().constData());
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            } else if (cur.tagName() == "playingsequences") {
                // Parse and add all playingsequence elements

                QDomElement temp = cur.firstChild().toElement();
                while (!temp.isNull()) {
                    if (temp.isElement()) {
                        int number = -1;
                        Playseq *playseq = Playseq::parse(temp);

                        if (playseq != NULL) {
                            prop = temp.attributeNode("number");
                            if (!prop.isNull()) {
                                number = prop.value().toInt();
                            }

                            while (song->playseqs.count() < number) {
                                song->playseqs.append(new Playseq);
                            }
                            if (song->playseqs.count() == number) {
                                song->playseqs.append(playseq);
                            } else {
                                delete song->playseqs.takeAt(number);
                                song->playseqs.insert(number, playseq);
                            }
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            } else if (cur.tagName() == "instruments") {
                // Parse and add all instrument elements
                QDomElement temp = cur.firstChild().toElement();

                while (!temp.isNull()) {
                    if (temp.isElement()) {
                        int number = -1;
                        Instrument *instrument = Instrument::parse(temp);

                        if (instrument != NULL) {
                            prop = temp.attributeNode("number");
                            if (!prop.isNull()) {
                                number = prop.value().toInt();
                            }

                            while (song->instruments.count() < number) {
                                song->instruments.append(new Instrument);
                            }
                            if (song->instruments.count() == number) {
                                song->instruments.append(instrument);
                            } else {
                                delete song->instruments.takeAt(number);
                                song->instruments.insert(number, instrument);
                            }
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            } else if (cur.tagName() == "tracks") {
                // Parse and add all track elements
                QDomElement temp = cur.firstChild().toElement();

                while (!temp.isNull()) {
                    if (temp.isElement()) {
                        int track = -1;

                        // The track number is required
                        if (temp.tagName() == "track") {
                            prop = temp.attributeNode("number");
                            if (!prop.isNull()) {
                                track = prop.value().toInt();
                            }

                            while (song->tracks.count() <= track) {
                                song->tracks.append(new Track);
                            }

                            // Get volume, mute, solo and name
                            prop = temp.attributeNode("volume");
                            if (!prop.isNull()) {
                                song->tracks[track]->setVolume(prop.value().toInt());
                            }

                            prop = temp.attributeNode("mute");
                            if (!prop.isNull()) {
                                song->tracks[track]->setMute(prop.value().toInt() > 0);
                            }

                            prop = temp.attributeNode("solo");
                            if (!prop.isNull()) {
                                song->tracks[track]->setSolo(prop.value().toInt() > 0);
                            }

                            song->tracks[track]->setName(temp.text());
                        } else if (temp.nodeType() != QDomNode::CommentNode) {
                            qWarning("XML error: expected section, got %s\n", temp.tagName().toUtf8().constData());
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            } else if (cur.tagName() == "trackvolumes") {
                // Backwards compatibility: parse and add all track volume elements
                QDomElement temp = cur.firstChild().toElement();

                while (!temp.isNull()) {
                    if (temp.isElement()) {
                        int track = -1;

                        // The track number is required
                        if (temp.tagName() == "trackvolume") {
                            prop = temp.attributeNode("track");
                            if (!prop.isNull()) {
                                track = prop.value().toInt();
                            }

                            while (song->tracks.count() < track) {
                                song->tracks.append(new Track);
                            }

                            // Get the volume
                            QDomElement temp2 = temp.firstChild().toElement();
                            if (!temp2.isNull()) {
                                song->tracks[track]->setVolume(temp2.text().toInt() & 127);
                                song->tracks[track]->setMute((temp2.text().toInt() & 128) > 0);
                            }
                        } else if (temp.nodeType() != QDomNode::CommentNode) {
                            qWarning("XML error: expected trackvolume, got %s\n", temp.tagName().toUtf8().constData());
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            } else if (cur.tagName() == "messages") {
                // Parse and add all Message elements
                QDomElement temp = cur.firstChild().toElement();

                while (!temp.isNull()) {
                    if (temp.isElement()) {
                        int number = -1;
                        Message *message = Message::parse(temp);

                        if (message != NULL) {
                            prop = temp.attributeNode("number");
                            if (!prop.isNull()) {
                                number = prop.value().toInt();
                            }

                            while (song->messages.count() < number) {
                                song->messages.append(new Message);
                            }
                            if (song->messages.count() == number) {
                                song->messages.append(message);
                            } else {
                                delete song->messages.takeAt(number);
                                song->messages.insert(number, message);
                            }
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            }
            cur = cur.nextSibling().toElement();
        }
    } else if (element.nodeType() != QDomNode::CommentNode) {
        qWarning("XML error: expected song, got %s\n", element.tagName().toUtf8().constData());
    }

    return song;
}

Block *Song::block(int number)
{
    return blocks[number];
}

Track *Song::track(int number)
{
    return tracks[number];
}

int Song::maxTracks() const
{
    return tracks.count();
}
