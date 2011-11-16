/*
 * playseq.cpp
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

#include <QDomElement>
#include "playseq.h"

Playseq::Playseq()
{
    blockNumbers.append(0);
}

Playseq::~Playseq()
{
}

void Playseq::insert(unsigned int pos)
{
    // Do not allow inserting beyond the block array size
    if (pos > blockNumbers.count()) {
        pos = blockNumbers.count();
    }

    // Which block number to insert
    blockNumbers.insert(pos, pos < blockNumbers.count() ? blockNumbers[pos] : blockNumbers[blockNumbers.count() - 1]);
}

void Playseq::remove(unsigned int pos)
{
    // Don't delete the last section
    if (blockNumbers.count() > 1) {
        blockNumbers.removeAt(pos);
    }
}

void Playseq::set(unsigned int pos, unsigned int block)
{
    blockNumbers[pos] = block;
}

unsigned int Playseq::at(unsigned int pos) const
{
    return blockNumbers[pos];
}

unsigned int Playseq::length() const
{
    return blockNumbers.length();
}

Playseq *Playseq::parse(QDomElement element)
{
    Playseq *playseq = NULL;

    if (element.tagName() == "playingsequence") {
        QDomAttr prop;

        // Allocate playseq
        playseq = new Playseq;
        prop = element.attributeNode("name");
        if (!prop.isNull()) {
            playseq->name = prop.value();
        }

        // Get playseq contents
        QDomElement cur = element.firstChild().toElement();
        while(!cur.isNull()) {
            if (cur.tagName() == "position") {
                int block = 0, number = 0;

                // Get the position
                prop = cur.attributeNode("number");
                if (!prop.isNull()) {
                    number = prop.value().toInt();
                }

                // Get block number
                block = cur.text().toInt();

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
