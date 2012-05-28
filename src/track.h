/*
 * track.h
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

#ifndef TRACK_H
#define TRACK_H

#include <QObject>

class Track : public QObject {
    Q_OBJECT

public:
    Track(const QString &name = QString(), QObject *parent = NULL);
    virtual ~Track();

    QString name() const;
    int volume() const;
    bool isMuted() const;
    bool isSolo() const;

public slots:
    void setName(const QString &name);
    void setVolume(int volume);
    void setMute(bool mute);
    void setSolo(bool solo);

signals:
    void mutedChanged(bool muted);
    void soloChanged(bool solo);

private:
    // Name of the track
    QString name_;
    // Track volume
    int volume_;
    // Mute toggle
    bool mute;
    // Solo toggle
    bool solo;
};

#endif // TRACK_H
