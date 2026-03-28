#ifndef DOLPHIN_OSTIME_H
#define DOLPHIN_OSTIME_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef s64 OSTime;
typedef u32 OSTick;

typedef struct OSCalendarTime {
    int sec;
    int min;
    int hour;
    int mday;
    int mon;   /* 0-11 */
    int year;
    int wday;
    int yday;
    int msec;
    int usec;
} OSCalendarTime;

#define OS_TIMER_CLOCK      40500000  // 40.5 MHz (bus / 4)
#define OS_BUS_CLOCK        162000000 // 162 MHz
#define OS_CORE_CLOCK       486000000 // 486 MHz

// Time conversion macros
#define OSSecondsToTicks(sec)           ((OSTime)(sec) * OS_TIMER_CLOCK)
#define OSMillisecondsToTicks(msec)     ((OSTime)(msec) * (OS_TIMER_CLOCK / 1000))
#define OSMicrosecondsToTicks(usec)     ((OSTime)(usec) * (OS_TIMER_CLOCK / 1000000))
#define OSNanosecondsToTicks(nsec)      ((OSTime)(nsec) / (1000000000 / OS_TIMER_CLOCK))

#define OSTicksToCycles(ticks)          ((OSTime)(ticks) * (OS_CORE_CLOCK / OS_TIMER_CLOCK))
#define OSTicksToSeconds(ticks)         ((ticks) / OS_TIMER_CLOCK)
#define OSTicksToMilliseconds(ticks)    ((ticks) / (OS_TIMER_CLOCK / 1000))
#define OSTicksToMicroseconds(ticks)    ((ticks) / (OS_TIMER_CLOCK / 1000000))
#define OSTicksToNanoseconds(ticks)     ((OSTime)(ticks) * (1000000000 / OS_TIMER_CLOCK))

OSTime OSGetTime(void);
OSTick OSGetTick(void);
OSTime OSGetSystemTime(void);
OSTime __OSGetSystemTime(void);  // Internal system time getter

void   OSTicksToCalendarTime(OSTime ticks, OSCalendarTime* timeDate);
OSTime OSCalendarTimeToTicks(const OSCalendarTime* timeDate);

#ifdef __cplusplus
}
#endif

#endif /* DOLPHIN_OSTIME_H */

