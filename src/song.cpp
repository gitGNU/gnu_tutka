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
    this->mute = mute;
}

bool Track::isMuted() const
{
    return mute;
}

void Track::setSolo(bool solo)
{
    this->solo = solo;
}

bool Track::isSolo() const
{
    return solo;
}

Song::Song(const QString &path, QObject *parent) :
    QObject(parent)
{
    bool initialized = false;

    if (!path.isEmpty()) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            if (doc.setContent(&file)) {
                initialized = parse(doc.documentElement());
            }
            file.close();
        }
    }

    if (!initialized) {
        init();
    }
}

Song::~Song()
{
    foreach(Playseq *playseq, playseqs_) {
        delete playseq;
    }
    foreach(Block *block, blocks_) {
        delete block;
    }
    foreach(Instrument *instrument, instruments_) {
        delete instrument;
    }
    foreach(Track *track, tracks) {
        delete track;
   }
    foreach(Message *message, messages_) {
        delete message;
    }
}

void Song::init()
{
    name_ = "Untitled";
    tempo_ = 120;
    ticksPerLine_ = 6;
    masterVolume_ = 127;
    sendSync_ = false;

    Block *block = new Block;
    connect(block, SIGNAL(tracksChanged(int)), this, SLOT(checkMaxTracks()));
    sections_.append(0);
    playseqs_.append(new Playseq);
    blocks_.append(block);
    checkMaxTracks();
}

void Song::insertBlock(unsigned int pos, unsigned int current)
{
    // Check block existence
    if (pos > blocks_.count()) {
        pos = blocks_.count();
    }
    if (current >= blocks_.count()) {
        current = blocks_.count() - 1;
    }

    // Insert a new block similar to the current block
    Block *block = new Block(blocks_[current]->tracks(), blocks_[current]->length(), blocks_[current]->commandPages());
    connect(block, SIGNAL(tracksChanged(int)), this, SLOT(checkMaxTracks()));
    blocks_.insert(pos, block);

    // Update playing sequences
    for (int i = 0; i < playseqs_.count(); i++) {
        for (unsigned int j = 0; j < playseqs_[i]->length(); j++) {
            if (playseqs_[i]->at(j) >= pos) {
                playseqs_[i]->set(j, playseqs_[i]->at(j) + 1);;
            }
        }
    }

    emit blocksChanged(blocks_.count());
}

void Song::deleteBlock(unsigned int pos)
{
    // Don't delete the last block
    if (blocks_.count() > 1) {
        // Check block existence
        if (pos >= blocks_.count()) {
            pos = blocks_.count() - 1;
        }

        blocks_.removeAt(pos);

        // Update playing sequences
        for (int i = 0; i < playseqs_.count(); i++) {
            for (int j = 0; j < playseqs_[i]->length(); j++) {
                if (playseqs_[i]->at(j) >= pos && playseqs_[i]->at(j) > 0) {
                    playseqs_[i]->set(j, playseqs_[i]->at(j) - 1);
                }
            }
        }
    }

    emit blocksChanged(blocks_.count());
}

// Inserts a new playseq in the playseq array in the given position
void Song::insertPlayseq(unsigned int pos)
{
    // Check playseq existence
    if (pos > playseqs_.count()) {
        pos = playseqs_.count();
    }

    // Insert a new playing sequence
    playseqs_.insert(pos, new Playseq);

    // Update sections
    for (int i = 0; i < sections_.count(); i++) {
        if (sections_[i] >= pos) {
            sections_[i]++;
        }
    }

    emit playseqsChanged(playseqs_.count());
}

void Song::deletePlayseq(unsigned int pos)
{
    // Don't delete the last playseq
    if (playseqs_.count() > 1) {
        // Check playseq existence
        if (pos >= playseqs_.count()) {
            pos = playseqs_.count() - 1;
        }

        playseqs_.removeAt(pos);

        // Update section lists
        for (int i = 0; i < sections_.count(); i++) {
            if (sections_[i] >= pos && sections_[i] > 0) {
                sections_[i]--;
            }
        }
    }

    emit playseqsChanged(playseqs_.count());
}

void Song::insertSection(unsigned int pos)
{
    // Check that the value is possible
    if (pos > sections_.count()) {
        pos = sections_.count();
    }

    sections_.insert(pos, pos < sections_.count() ? sections_[pos] : sections_[sections_.count() - 1]);

    emit sectionsChanged(sections_.count());
}

void Song::deleteSection(unsigned int pos)
{
    // Don't delete the last section
    if (sections_.count() > 1) {
        // Check section existence
        if (pos >= sections_.count()) {
            pos = sections_.count() - 1;
        }

        sections_.removeAt(pos);
    }

    emit sectionsChanged(sections_.count());
}

void Song::insertMessage(unsigned int pos)
{
    // Check message existence
    if (pos > messages_.count()) {
        pos = messages_.count();
    }

    // Insert a new message
    messages_.insert(pos, new Message());

    emit messagesChanged(messages_.count());
}

void Song::deleteMessage(unsigned int pos)
{
    // Don't delete inexisting messages
    if (messages_.count() > 0) {
        // Check message existence
        if (pos >= messages_.count()) {
            pos = messages_.count() - 1;
        }

        // Free the message in question
        messages_.removeAt(pos);
    }

    emit messagesChanged(messages_.count());
}

void Song::setSection(unsigned int pos, unsigned int playseq)
{
    if (pos < sections_.count() && playseq < playseqs_.count()) {
        sections_[pos] = playseq;
    }
}

void Song::setTPL(int ticksPerLine)
{
    ticksPerLine_ = ticksPerLine;
}

void Song::setTempo(int tempo)
{
    tempo_ = tempo;
}

void Song::checkMaxTracks()
{
    int max = 0;
    int oldMax = tracks.count();

    // Check the maximum number of tracks;
    for (int track = 0; track < blocks_.count(); track++) {
        if (blocks_[track]->tracks() > max) {
            max = blocks_[track]->tracks();
        }
    }

    if (oldMax < max) {
        for (int track = oldMax; track < max; track++) {
            // Give a descriptive name for the new track
            tracks.append(new Track(QString("Track %1").arg(track + 1)));
        }
    } else if (oldMax > max) {
        // Tracks removed: free track datas
        while (tracks.count() > max) {
            delete tracks.takeLast();
        }
     }

    if (max != oldMax) {
        emit maxTracksChanged(max);
    }
}

void Song::checkInstrument(int instrument, unsigned short defaultMIDIInterface)
{
    while (instrument >= instruments_.count()) {
        instruments_.append(new Instrument("Unnamed", defaultMIDIInterface));
    }
}

void Song::transpose(int instrument, int halfNotes)
{
    for (int block = 0; block < blocks_.count(); block++) {
        blocks_[block]->transpose(instrument, halfNotes, 0, 0, blocks_[block]->tracks() - 1, blocks_[block]->length() - 1);
    }
}

void Song::expandShrink(int factor)
{
    for (int block = 0; block < blocks_.count(); block++) {
        blocks_[block]->expandShrink(factor, 0, 0, blocks_[block]->tracks() - 1, blocks_[block]->length() - 1);
    }
}

void Song::changeInstrument(int from, int to, bool swap)
{
    for (int block = 0; block < blocks_.count(); block++) {
        blocks_[block]->changeInstrument(from, to, swap, 0, 0, blocks_[block]->tracks() - 1, blocks_[block]->length() - 1);
    }
}

bool Song::parse(QDomElement element)
{
    if (element.tagName() == "song") {
        QDomAttr prop;

        prop = element.attributeNode("name");
        if (!prop.isNull()) {
            name_ = prop.value();
        }

        prop = element.attributeNode("tempo");
        if (!prop.isNull()) {
            tempo_ = prop.value().toInt();
        }

        prop = element.attributeNode("ticksperline");
        if (!prop.isNull()) {
            ticksPerLine_ = prop.value().toInt();
        }

        prop = element.attributeNode("mastervolume");
        if (!prop.isNull()) {
            masterVolume_ = prop.value().toInt();
        }

        prop = element.attributeNode("sendsync");
        if (!prop.isNull()) {
            sendSync_ = (prop.value().toInt() == 1);
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

                            while (blocks_.count() < number) {
                                blocks_.append(new Block);
                            }
                            if (blocks_.count() == number) {
                                blocks_.append(block);
                            } else {
                                delete blocks_.takeAt(number);
                                blocks_.insert(number, block);
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
                            while (sections_.count() < number) {
                                sections_.append(0);
                            }
                            if (sections_.count() == number) {
                                sections_.append(temp.text().toInt());
                            } else {
                                sections_.replace(number, temp.text().toInt());
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

                            while (playseqs_.count() < number) {
                                playseqs_.append(new Playseq);
                            }
                            if (playseqs_.count() == number) {
                                playseqs_.append(playseq);
                            } else {
                                delete playseqs_.takeAt(number);
                                playseqs_.insert(number, playseq);
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

                            while (instruments_.count() < number) {
                                instruments_.append(new Instrument);
                            }
                            if (instruments_.count() == number) {
                                instruments_.append(instrument);
                            } else {
                                delete instruments_.takeAt(number);
                                instruments_.insert(number, instrument);
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

                            while (tracks.count() <= track) {
                                tracks.append(new Track);
                            }

                            // Get volume, mute, solo and name
                            prop = temp.attributeNode("volume");
                            if (!prop.isNull()) {
                                tracks[track]->setVolume(prop.value().toInt());
                            }

                            prop = temp.attributeNode("mute");
                            if (!prop.isNull()) {
                                tracks[track]->setMute(prop.value().toInt() > 0);
                            }

                            prop = temp.attributeNode("solo");
                            if (!prop.isNull()) {
                                tracks[track]->setSolo(prop.value().toInt() > 0);
                            }

                            tracks[track]->setName(temp.text());
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

                            while (tracks.count() < track) {
                                tracks.append(new Track);
                            }

                            // Get the volume
                            QDomElement temp2 = temp.firstChild().toElement();
                            if (!temp2.isNull()) {
                                tracks[track]->setVolume(temp2.text().toInt() & 127);
                                tracks[track]->setMute((temp2.text().toInt() & 128) > 0);
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

                            while (messages_.count() < number) {
                                messages_.append(new Message);
                            }
                            if (messages_.count() == number) {
                                messages_.append(message);
                            } else {
                                delete messages_.takeAt(number);
                                messages_.insert(number, message);
                            }
                        }
                    }

                    temp = temp.nextSibling().toElement();
                }
            }
            cur = cur.nextSibling().toElement();
        }
        return true;
    } else {
        qWarning("XML error: expected song, got %s\n", element.tagName().toUtf8().constData());
        return false;
    }
}

Block *Song::block(unsigned int number) const
{
    return number < blocks_.count() ? blocks_[number] : NULL;
}

Track *Song::track(unsigned int number) const
{
    return number < tracks.count() ? tracks[number] : NULL;
}

Playseq *Song::playseq(unsigned int number) const
{
    return number < playseqs_.count() ? playseqs_[number] : NULL;
}

Instrument *Song::instrument(unsigned int number) const
{
    return number < instruments_.count() ? instruments_[number] : NULL;
}

Message *Song::message(unsigned int number) const
{
    return number < messages_.count() ? messages_[number] : NULL;
}

unsigned int Song::maxTracks() const
{
    return tracks.count();
}

unsigned int Song::blocks() const
{
    return blocks_.count();
}

unsigned int Song::playseqs() const
{
    return playseqs_.count();
}

unsigned int Song::sections() const
{
    return sections_.count();
}

unsigned int Song::instruments() const
{
    return instruments_.count();
}

unsigned int Song::messages() const
{
    return messages_.count();
}

unsigned int Song::section(unsigned int pos) const
{
    return sections_[pos];
}

unsigned int Song::masterVolume() const
{
    return masterVolume_;
}

unsigned int Song::tempo() const
{
    return tempo_;
}

unsigned int Song::ticksPerLine() const
{
    return ticksPerLine_;
}

bool Song::sendSync() const
{
    return sendSync_;
}

QString Song::name() const
{
    return name_;
}

void Song::setName(const QString &name)
{
    name_ = name;
}

void Song::setSendSync(bool sendSync)
{
    sendSync_ = sendSync;
}
