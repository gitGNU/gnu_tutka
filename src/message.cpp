/*
 * message.cpp
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

#include <QFile>
#include <QDomElement>
#include "message.h"

Message::Message() :
    autoSend(false)
{
}

Message::~Message()
{
}

bool Message::isAutoSend() const
{
    return autoSend;
}

unsigned int Message::length() const
{
    return data.length();
}

const char *Message::rawData() const
{
    return data.constData();
}

void Message::setLength(unsigned int length)
{
    data.resize(length);
}

void Message::setAutoSend(bool autoSend)
{
    this->autoSend = autoSend;
}

void Message::loadBinary(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        data.resize(file.size());
        file.read(data.data(), file.size());
    }
}

void Message::saveBinary(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
    }
}

Message *Message::parse(QDomElement element)
{
    Message *message = NULL;

    if (element.tagName() == "message") {
        QDomAttr prop;
        char c[3];

        // Temporary string for hexadecimal parsing
        c[2] = 0;

        // Allocate message
        message = new Message;
        prop = element.attributeNode("name");
        if (!prop.isNull()) {
            message->name = prop.value();
        }

        prop = element.attributeNode("autosend");
        if (!prop.isNull()) {
            message->autoSend = prop.value().toInt() > 0;
        }

        int length = element.text().length() / 2;
        message->data.resize(length);
        unsigned int d;
        for (int i = 0; i < length; i++) {
            c[0] = element.text().at(i * 2).toAscii();
            c[1] = element.text().at(i * 2 + 1).toAscii();
            sscanf((char *)c, "%X", &d);
            message->data[i] = d;
            printf("WTF %x\n", d);
        }
    } else if (element.nodeType() != QDomNode::CommentNode) {
        qWarning("XML error: expected message, got %s\n", element.tagName().toUtf8().constData());
    }
    return message;
}

void Message::save(int number, QDomElement &parentElement, QDomDocument &document)
{
    QString message;
    for (int i = 0; i < data.length(); i++) {
        message += QString("%1").arg(data.constData()[i] & 0xff, 2, 16, QChar('0'));
    }

    QDomElement messageElement = document.createElement("message");
    parentElement.appendChild(messageElement);
    messageElement.appendChild(document.createTextNode(message.toUpper()));
    messageElement.setAttribute("number", number);
    messageElement.setAttribute("name", name);
    messageElement.setAttribute("autosend", autoSend ? 1 : 0);
    parentElement.appendChild(document.createTextNode("\n"));
}
