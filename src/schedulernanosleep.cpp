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
