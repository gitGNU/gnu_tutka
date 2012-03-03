/*
 * smf.cpp
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

#include <QFile>
#include "smf.h"

SMF::SMF()
{
}

void SMF::addTrack(const QByteArray &data)
{
    tracks.append(SMFTrack(data));
}

void SMF::save(const QString &path) const
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    file.write(header.toByteArray());
    foreach(const SMFTrack &track, tracks) {
        file.write(track.toByteArray());
    }
}

SMFChunk::SMFChunk(unsigned int id) : id(id)
{
}

QByteArray SMFChunk::toByteArray() const
{
    QByteArray array;
    array.append(id >> 24);
    array.append(id >> 16);
    array.append(id >> 8);
    array.append(id);
    array.append(data.length() >> 24);
    array.append(data.length() >> 16);
    array.append(data.length() >> 8);
    array.append(data.length());
    array.append(data);
    return array;
}

SMFHeader::SMFHeader(unsigned short format, unsigned short tracks, unsigned short division) :
    SMFChunk('MThd')
{
    data.append(format >> 8);
    data.append(format);
    data.append(tracks >> 8);
    data.append(tracks);
    data.append(division >> 8);
    data.append(division);
}

SMFTrack::SMFTrack(const QByteArray &data) :
    SMFChunk('MTrk')
{
    this->data = data;
}
