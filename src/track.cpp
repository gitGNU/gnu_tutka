/*
 * track.cpp
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

#include "track.h"

Track::Track(const QString &name, QObject *parent) :
    QObject(parent),
    name_(name),
    volume_(127),
    mute(false),
    solo(false)
{
}

Track::~Track()
{
}

void Track::setName(const QString &name)
{
    if (name_ != name) {
        name_ = name;

        emit nameChanged(name_);
    }
}

QString Track::name() const
{
    return name_;
}

void Track::setVolume(int volume)
{
    volume_ = volume;
}

int Track::volume() const
{
    return volume_;
}

void Track::setMute(bool mute)
{
    if (this->mute != mute) {
        this->mute = mute;

        emit mutedChanged(this->mute);
    }
}

bool Track::isMuted() const
{
    return mute;
}

void Track::setSolo(bool solo)
{
    if (this->solo != solo) {
        this->solo = solo;

        emit soloChanged(this->solo);
    }
}

bool Track::isSolo() const
{
    return solo;
}

