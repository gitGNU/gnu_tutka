/*
 * message.h
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

#ifndef MESSAGE_H_
#define MESSAGE_H_

class QDomElement;

class Message {
public:
    Message();
    virtual ~Message();

    // Sets the length of a MIDI message
    void setLength(unsigned int);
    // Sets the auto send flag of a MIDI message
    void setAutoSend(bool);
    // Parses a element in an XML file
    static Message *parse(QDomElement element);
    // Saves a message to an XML file
    //void save(int, xmlNodePtr);
    // Loads a message from a file
    void loadBinary(const QString &filename);
    // Saves a message to a file
    void saveBinary(const QString &filename);

    bool isAutoSend() const;
    unsigned int length() const;
    const char *rawData() const;

private:
    // Name
    QString name;
    // Dump data
    QByteArray data;
    // Automatically send after loading?
    bool autoSend;
};

#endif /* MESSAGE_H_ */
