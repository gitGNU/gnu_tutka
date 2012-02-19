/*
 * playseq.h
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

#ifndef PLAYSEQ_H_
#define PLAYSEQ_H_

#include <QObject>
#include <QString>
#include <QList>

class QDomElement;

class Playseq : public QObject {
    Q_OBJECT

public:
    Playseq(QObject *parent = NULL);
    virtual ~Playseq();

    // Returns the length of the playing sequence
    unsigned int length() const;
    // Returns the block at the given position
    unsigned int at(unsigned int pos) const;
    // Sets the block at the given position
    void set(unsigned int pos, unsigned int block);
    // Inserts a new block in the block array in the given position
    void insert(unsigned int pos);
    // Deletes a block from the given position of the block array
    void remove(unsigned int pos);
    //void save(int, xmlNodePtr);
    // Parses a playingsequence element in an XML file
    static Playseq *parse(QDomElement element);

signals:
    void playseqChanged();

private:
    // Name
    QString name;
    // Block number array
    QList<unsigned int> blockNumbers;
};

#endif /* PLAYSEQ_H_ */
