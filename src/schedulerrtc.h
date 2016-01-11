/*
 * schedulerrtc.h
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

#ifndef SCHEDULERRTC_H
#define SCHEDULERRTC_H

#include "scheduler.h"

class SchedulerRTC : public Scheduler
{
    Q_OBJECT

public:
    virtual const char *name() const;
    virtual void waitForTick(Song *song, bool schedulerChanged);
    virtual void stop();

private:
    explicit SchedulerRTC(QObject *parent = 0);
    virtual ~SchedulerRTC();

    static SchedulerRTC instance;

    // RTC device file descriptor
    int rtc;
    // Obtained RTC frequency
    int rtcFrequency;
    // RTC periodic interrupts enabled
    bool rtcPIE;
};

#endif // SCHEDULERRTC_H
