/*
 * scheduler.cpp
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

#include <sys/time.h>
#include "song.h"
#include "scheduler.h"

QList<Scheduler *> Scheduler::schedulers_;

Scheduler::Scheduler(QObject *parent) :
    QObject(parent)
{
}

Scheduler::~Scheduler()
{
}

void Scheduler::start(struct timeval &startTime)
{
    next.tv_sec = startTime.tv_sec;
    next.tv_usec = startTime.tv_usec;
}

void Scheduler::waitForTick(Song *song, bool schedulerChanged)
{
    // If the scheduler has changed from external sync reset the timing
    if (schedulerChanged) {
        gettimeofday(&next, NULL);
    }

    // Calculate time of next tick (tick every 1000000/((BPM/60)*24) usecs)
    next.tv_sec += (25 / song->tempo()) / 10;
    next.tv_usec += ((2500000 / song->tempo()) % 1000000);
    while (next.tv_usec > 1000000) {
        next.tv_sec++;
        next.tv_usec -= 1000000;
    }

    gettimeofday(&now, NULL);
}

void Scheduler::stop()
{
}

QList<Scheduler *> Scheduler::schedulers()
{
    return schedulers_;
}
