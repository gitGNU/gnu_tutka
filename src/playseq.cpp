/*
 * playseq.cpp
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

#include <QDomElement>
#include "playseq.h"

Playseq::Playseq(QObject *parent) : QObject(parent)
{
    blockNumbers.append(0);
}

Playseq::~Playseq()
{
}

QString Playseq::name() const
{
    return name_;
}

void Playseq::setName(const QString &name)
{
    name_ = name;

    emit nameChanged(name);
}

unsigned int Playseq::length() const
{
    return blockNumbers.length();
}

unsigned int Playseq::at(unsigned int pos) const
{
    return blockNumbers[pos < blockNumbers.count() ? pos : (blockNumbers.count() - 1)];
}

void Playseq::set(unsigned int pos, unsigned int block)
{
    blockNumbers[pos < blockNumbers.count() ? pos : (blockNumbers.count() - 1)] = block;

    emit blocksChanged();
}

void Playseq::insert(unsigned int pos)
{
    // Do not allow inserting beyond the block array size
    if (pos > blockNumbers.count()) {
        pos = blockNumbers.count();
    }

    // Which block number to insert
    blockNumbers.insert(pos, blockNumbers[pos < blockNumbers.count() ? pos : (blockNumbers.count() - 1)]);

    emit lengthChanged();
}

void Playseq::remove(unsigned int pos)
{
    // Don't delete the last section
    if (blockNumbers.count() > 1) {
        blockNumbers.removeAt(pos);
    }

    emit lengthChanged();
}

Playseq *Playseq::parse(QDomElement element)
{
    Playseq *playseq = NULL;

    if (element.tagName() == "playingsequence") {
        // Allocate playseq
        playseq = new Playseq;
        QDomAttr prop = element.attributeNode("name");
        if (!prop.isNull()) {
            playseq->name_ = prop.value();
        }

        // Get playseq contents
        QDomElement cur = element.firstChild().toElement();
        while(!cur.isNull()) {
            if (cur.tagName() == "position") {
                int number = 0;

                // Get the position
                prop = cur.attributeNode("number");
                if (!prop.isNull()) {
                    number = prop.value().toInt();
                }

                // Get block number
                int block = cur.text().toInt();

                while (playseq->blockNumbers.count() < number) {
                    playseq->blockNumbers.append(0);
                }
                if (playseq->blockNumbers.count() == number) {
                    playseq->blockNumbers.append(block);
                } else {
                    playseq->blockNumbers.replace(number, block);
                }
            }
            cur = cur.nextSibling().toElement();
        }
    } else if (element.nodeType() != QDomNode::CommentNode) {
        qWarning("XML error: expected playseq, got %s\n", element.tagName().toUtf8().constData());
    }

    return playseq;
}

void Playseq::save(int number, QDomElement &parentElement, QDomDocument &document)
{
    QDomElement playingSequenceElement = document.createElement("playingsequence");
    parentElement.appendChild(playingSequenceElement);
    playingSequenceElement.setAttribute("number", number);
    playingSequenceElement.setAttribute("name", name_);
    playingSequenceElement.appendChild(document.createTextNode("\n"));

    // Add all blocks
    for (int position = 0; position < blockNumbers.count(); position++) {
        QDomElement positionElement = document.createElement("position");
        positionElement.appendChild(document.createTextNode(QString("%1").arg(blockNumbers[position])));
        positionElement.setAttribute("number", position);
        playingSequenceElement.appendChild(positionElement);
        playingSequenceElement.appendChild(document.createTextNode("\n"));
    }

    parentElement.appendChild(document.createTextNode("\n"));
}
