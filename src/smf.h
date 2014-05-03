/*
 * smf.h
 *
 * Copyright 2002-2014 Vesa Halttunen
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

#ifndef SMF_H
#define SMF_H

#include <QByteArray>
#include <QList>

class SMFChunk {
public:
    SMFChunk(unsigned int id);
    QByteArray toByteArray() const;

protected:
    unsigned int id;
    QByteArray data;
};

class SMFHeader : public SMFChunk {
public:
    SMFHeader(unsigned short format = 0, unsigned short tracks = 1, unsigned short division = 24);
};

class SMFTrack : public SMFChunk {
public:
    SMFTrack(const QByteArray &data);
};

class SMF
{
public:
    SMF();

    void addTrack(const QByteArray &data);
    void save(const QString &path) const;

private:
    SMFHeader header;
    QList<SMFTrack> tracks;
};

#endif // SMF_H
