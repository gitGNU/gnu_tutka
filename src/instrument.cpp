/*
 * instrument.cpp
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
#include "block.h"
#include "instrument.h"

Instrument::Instrument(const QString &name, unsigned int midiInterface) :
    name_(name),
    midiInterface_(midiInterface),
    midiPreset_(0),
    midiChannel_(0),
    defaultVelocity_(127),
    transpose_(0),
    hold_(0),
    arpeggio_(NULL),
    basenote_(0)
{
}

Instrument::~Instrument()
{
}

Instrument *Instrument::parse(QDomElement element)
{
    Instrument *instrument = NULL;

    if (element.tagName() == "instrument") {
        QDomAttr prop;

        // Allocate instrument
        instrument = new Instrument;
        prop = element.attributeNode("name");
        if (!prop.isNull()) {
            instrument->name_ = prop.value();
        }

        // Get instrument contents
        QDomElement cur = element.firstChild().toElement();
        while(!cur.isNull()) {
            if (cur.tagName() == "output") {
                // Get output properties
                prop = cur.attributeNode("midiinterface");
                if (!prop.isNull()) {
                    instrument->midiInterfaceName_ = prop.value();
                }

                prop = cur.attributeNode("midipreset");
                if (!prop.isNull()) {
                    instrument->midiPreset_ = prop.value().toInt();
                }

                prop = cur.attributeNode("midichannel");
                if (!prop.isNull()) {
                    instrument->midiChannel_ = prop.value().toInt();
                }

                prop = cur.attributeNode("defaultvelocity");
                if (!prop.isNull()) {
                    instrument->defaultVelocity_ = prop.value().toInt();
                }

                prop = cur.attributeNode("transpose");
                if (!prop.isNull()) {
                    instrument->transpose_ = prop.value().toInt();
                }

                prop = cur.attributeNode("hold");
                if (!prop.isNull()) {
                    instrument->hold_ = prop.value().toInt();
                }
            } else if (cur.tagName() == "arpeggio") {
                // Get arpeggio properties
                prop = cur.attributeNode("basenote");
                if (!prop.isNull()) {
                    instrument->basenote_ = prop.value().toInt();
                }

                // Parse and add all block elements
                for (QDomElement temp = cur.firstChild().toElement(); !temp.isNull() && instrument->arpeggio_ == NULL; temp = temp.nextSibling().toElement()) {
                    if (temp.isElement()) {
                        instrument->arpeggio_ = Block::parse(temp);
                    }
                }
            }
            cur = cur.nextSibling().toElement();
        }
    } else if (element.nodeType() != QDomNode::CommentNode) {
        qWarning("XML error: expected instrument, got %s\n", element.tagName().toUtf8().constData());
    }

    return instrument;
}

QString Instrument::name() const
{
    return name_;
}

unsigned int Instrument::midiInterface() const
{
    return midiInterface_;
}

QString Instrument::midiInterfaceName() const
{
    return midiInterfaceName_;
}

unsigned short Instrument::midiPreset() const
{
    return midiPreset_;
}

unsigned char Instrument::midiChannel() const
{
    return midiChannel_;
}

unsigned char Instrument::defaultVelocity() const
{
    return defaultVelocity_;
}

char Instrument::transpose() const
{
    return transpose_;
}

unsigned char Instrument::hold() const
{
    return hold_;
}

Block *Instrument::arpeggio() const
{
    return arpeggio_;
}

unsigned char Instrument::basenote() const
{
    return basenote_;
}
