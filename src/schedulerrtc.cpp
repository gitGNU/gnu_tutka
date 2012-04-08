#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#ifdef __linux
#include <linux/rtc.h>
#endif
#include "schedulerrtc.h"

#define MAXIMUM_RTC_FREQ 2048
#define MINIMUM_RTC_FREQ 512

SchedulerRTC::SchedulerRTC(QObject *parent) :
    Scheduler(parent),
    rtc(-1),
    rtcFrequency(0),
    rtcPIE(false)
{
    bool success = false;

    rtc = open("/dev/rtc", O_RDONLY);
    if (rtc != -1) {
        rtcFrequency = MAXIMUM_RTC_FREQ;
        rtcPIE = 0;

#ifdef __linux
        // RTC scheduling requested: try every 2^N value down from MAXIMUM_RTC_FREQ to MINIMUM_RTC_FREQ
        while (rtcFrequency >= MINIMUM_RTC_FREQ && !success) {
            if (ioctl(rtc, RTC_UIE_OFF, 0) == -1 || ioctl(rtc, RTC_IRQP_SET, rtcFrequency) == -1) {
                rtcFrequency /= 2;
            } else {
                success = true;
            }
        }
#endif

        // If all frequencies failed fall back to nanosleep()
        if (!success) {
            close(rtc);
            rtc = -1;
        }
    }

    if (success) {
        schedulers_.append(this);
    }
}

SchedulerRTC::~SchedulerRTC()
{
    // Close RTC
    if (rtc != -1) {
        close(rtc);
    }
}

const char *SchedulerRTC::name() const
{
    return "RTC";
}

void SchedulerRTC::waitForTick(Song *song, bool schedulerChanged)
{
    Scheduler::waitForTick(song, schedulerChanged);

    if (rtc != -1) {
        // Make sure periodic interrupts are on
        if (!rtcPIE) {
    #ifdef __linux
            ioctl(rtc, RTC_PIE_ON, 0);
    #endif
            rtcPIE = true;
        }

        // Rtc: read RTC interrupts until it's time to play again
        while ((next.tv_sec - now.tv_sec) > 0 || ((next.tv_sec - now.tv_sec) == 0 && (next.tv_usec - now.tv_usec) > 1000000 / rtcFrequency)) {
            unsigned long l;
            ssize_t n = read(rtc, &l, sizeof(unsigned long));
            Q_UNUSED(n)
            gettimeofday(&now, NULL);
        }
        while ((next.tv_sec - now.tv_sec) > 0 || ((next.tv_sec - now.tv_sec) == 0 && (next.tv_usec - now.tv_usec) > 0)) {
            gettimeofday(&now, NULL);
        }
    }
}

void SchedulerRTC::stop()
{
    // Turn off periodic interrupts
    if (rtcPIE && rtc != -1) {
#ifdef __linux
        ioctl(rtc, RTC_PIE_OFF, 0);
#endif
        rtcPIE = false;
    }
}

SchedulerRTC SchedulerRTC::instance;
