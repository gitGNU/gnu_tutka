/*
 * buffermidiinterface.cpp
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

#include "buffermidi.h"
#include "buffermidiinterface.h"

BufferMIDIInterface::BufferMIDIInterface(BufferMIDI *midi, DirectionFlags flags, QObject *parent) :
    MIDIInterface(flags, parent),
    midi(midi)
{
}

void BufferMIDIInterface::write(const QByteArray &dataArray)
{
    int length = dataArray.length();
    const char *data = dataArray.constData();
    if (length > 0 && data[0] == (char)0xf0) {
        // SysEx messages need special treatment if written to a file
        unsigned long value = length - 1;
        unsigned long varlen = value & 0x7F;
        int i, l;

        // Create a variable length version of the length
        while ((value >>= 7)) {
            varlen <<= 8;
            varlen |= ((value & 0x7F) | 0x80);
        }

        // Check how many bytes the varlen version requires
        value = varlen;
        for (l = 1; l <= 4; l++) {
            if (value & 0x80) {
                value >>= 8;
            } else {
                break;
            }
        }

        // Allocate buffer
        char *newmessage = new char[length + l];
        newmessage[0] = 0xf0;

        // Write the varlen length
        for (i = 1; i <= 4; i++) {
            newmessage[i] = (unsigned char)(varlen & 0xff);
            if (varlen & 0x80) {
                varlen >>= 8;
            } else {
                break;
            }
        }

        // Copy the rest of the data
        memcpy(newmessage + i + 1, data + 1, length - 1);
        write(QByteArray(newmessage, length + l));
        delete [] newmessage;
    } else {
        data_.append(data, length);
    }
}

QByteArray BufferMIDIInterface::data() const
{
    return data_;
}
