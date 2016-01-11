/*
 * message.cpp
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

#include <QFile>
#include <QDomElement>
#include "message.h"

Message::Message(QObject *parent) :
    QObject(parent),
    autoSend(false)
{
}

Message::~Message()
{
}

QString Message::name() const
{
    return name_;
}

void Message::setName(const QString &name)
{
    name_ = name;
}

unsigned int Message::length() const
{
    return data_.length();
}

void Message::setLength(unsigned int length)
{
    int oldLength = data_.length();

    if (length != oldLength) {
        data_.resize(length);

        for (int i = oldLength; i < length; i++) {
            data_[i] = 0;
        }

        emit lengthChanged();
    }
}

bool Message::isAutoSend() const
{
    return autoSend;
}

void Message::setAutoSend(bool autoSend)
{
    this->autoSend = autoSend;
}

QByteArray Message::data() const
{
    return data_;
}

void Message::setData(const QByteArray &data)
{
    int oldLength = data_.length();

    data_ = data;

    if (data_.length() != oldLength) {
        emit lengthChanged();
    }
}

void Message::loadBinary(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        data_.resize(file.size());
        file.read(data_.data(), file.size());
    }
}

void Message::saveBinary(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data_);
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
            message->name_ = prop.value();
        }

        prop = element.attributeNode("autosend");
        if (!prop.isNull()) {
            message->autoSend = prop.value().toInt() > 0;
        }

        int length = element.text().length() / 2;
        message->data_.resize(length);
        unsigned int d;
        QByteArray data = element.text().toLatin1();
        for (int i = 0; i < length; i++) {
            c[0] = data.at(i * 2);
            c[1] = data.at(i * 2 + 1);
            sscanf((char *)c, "%X", &d);
            message->data_[i] = d;
        }
    } else if (element.nodeType() != QDomNode::CommentNode) {
        qWarning("XML error: expected message, got %s\n", element.tagName().toUtf8().constData());
    }
    return message;
}

void Message::save(int number, QDomElement &parentElement, QDomDocument &document)
{
    QString message;
    for (int i = 0; i < data_.length(); i++) {
        message += QString("%1").arg(data_.constData()[i] & 0xff, 2, 16, QChar('0'));
    }

    QDomElement messageElement = document.createElement("message");
    parentElement.appendChild(messageElement);
    messageElement.appendChild(document.createTextNode(message.toUpper()));
    messageElement.setAttribute("number", number);
    messageElement.setAttribute("name", name_);
    messageElement.setAttribute("autosend", autoSend ? 1 : 0);
    parentElement.appendChild(document.createTextNode("\n"));
}
