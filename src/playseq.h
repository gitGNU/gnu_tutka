/*
 * playseq.h
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

#ifndef PLAYSEQ_H_
#define PLAYSEQ_H_

#include <QObject>
#include <QString>
#include <QList>

class QDomElement;
class QDomDocument;

class Playseq : public QObject {
    Q_OBJECT

public:
    Playseq(QObject *parent = NULL);
    virtual ~Playseq();

    // Returns the name of the playing sequence
    QString name() const;

    // Sets the name of the playing sequence
    void setName(const QString &name);

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

    // Parses a playingsequence element in an XML file
    static Playseq *parse(QDomElement element);

    // Saves the playing sequence to an XML document
    void save(int number, QDomElement &parentElement, QDomDocument &document);

signals:
    // Emitted when the playing sequence contents change
    void playseqChanged();

    // Emitted when the playing sequence name changes
    void nameChanged(QString);

private:
    // Name
    QString name_;
    // Block number array
    QList<unsigned int> blockNumbers;
};

#endif /* PLAYSEQ_H_ */
