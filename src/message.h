/*
 * message.h
 *
 * Copyright 2002-2012 Vesa Halttunen
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

#ifndef MESSAGE_H_
#define MESSAGE_H_

class QDomElement;
class QDomDocument;

class Message {
public:
    Message();
    virtual ~Message();

    // Returns whether the message should be automatically sent after loading
    bool isAutoSend() const;
    // Returns the length of the message
    unsigned int length() const;
    // Returns a the data of the message
    QByteArray data() const;
    // Sets the length of a MIDI message
    void setLength(unsigned int);
    // Sets the auto send flag of a MIDI message
    void setAutoSend(bool);
    // Loads a message from a file
    void loadBinary(const QString &filename);
    // Saves a message to a file
    void saveBinary(const QString &filename);
    // Parses a element in an XML file
    static Message *parse(QDomElement element);
    // Saves a message to an XML file
    void save(int number, QDomElement &parentElement, QDomDocument &document);
    QString name() const;
    void setName(const QString &name);

private:
    // Name
    QString name_;
    // Dump data
    QByteArray data_;
    // Automatically send after loading?
    bool autoSend;
};

#endif /* MESSAGE_H_ */
