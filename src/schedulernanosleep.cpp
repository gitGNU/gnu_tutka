/*
 * schedulernanosleep.cpp
 *
 * Copyright 2002-2016 Vesa Halttunen
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

#include "schedulernanosleep.h"

SchedulerNanoSleep::SchedulerNanoSleep(QObject *parent) :
    Scheduler(parent)
{
    schedulers_.append(this);
}

const char *SchedulerNanoSleep::name() const
{
    return "NanoSleep";
}

void SchedulerNanoSleep::waitForTick(Song *song, bool schedulerChanged)
{
    Scheduler::waitForTick(song, schedulerChanged);

    // Nanosleep: calculate difference between now and the next tick
    struct timespec req, rem;
    req.tv_sec = next.tv_sec - now.tv_sec;
    req.tv_nsec = (next.tv_usec - now.tv_usec) * 1000;
    while (req.tv_nsec < 0) {
        req.tv_sec--;
        req.tv_nsec += 1000000000;
    }

    // Sleep until the next tick if necessary
    if (req.tv_sec >= 0) {
        while (nanosleep(&req, &rem) == -1) { };
    }
}

SchedulerNanoSleep SchedulerNanoSleep::instance;
