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
