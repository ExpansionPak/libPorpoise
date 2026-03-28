/*---------------------------------------------------------------------------*
  OSAlarm.c - Alarm/Timer System Implementation
  
  On original hardware, alarms use the PowerPC Decrementer Exception (hardware
  timer interrupt) to fire callbacks at scheduled times. The alarm queue is
  sorted by fire time, and the hardware timer is set to fire for the next alarm.
  
  On PC, we simulate this with:
  - A sorted linked list queue of pending alarms
  - A background thread that sleeps until the next alarm
  - Mutex protection for thread-safe queue access
  - Support for one-shot and periodic alarms
 *---------------------------------------------------------------------------*/

#include <dolphin/os.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

/*---------------------------------------------------------------------------*
  Alarm Queue Structure
  
  Maintains a doubly-linked list of alarms sorted by fire time (earliest first).
  This matches the original hardware implementation.
 *---------------------------------------------------------------------------*/
static struct OSAlarmQueue {
    OSAlarm* head;
    OSAlarm* tail;
} s_alarmQueue = { NULL, NULL };

/* Thread synchronization */
#ifdef _WIN32
static CRITICAL_SECTION s_alarmMutex;
static HANDLE s_alarmThread = NULL;
static HANDLE s_alarmEvent = NULL;
static volatile BOOL s_alarmThreadRunning = FALSE;
#else
static pthread_mutex_t s_alarmMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_alarmThread;
static pthread_cond_t s_alarmCond = PTHREAD_COND_INITIALIZER;
static volatile BOOL s_alarmThreadRunning = FALSE;
#endif

static BOOL s_alarmSystemInitialized = FALSE;

/*---------------------------------------------------------------------------*
  Forward declarations
 *---------------------------------------------------------------------------*/
static void InsertAlarm(OSAlarm* alarm, OSTime fire, OSAlarmHandler handler);
static void* AlarmThreadFunc(void* arg);
static void LockAlarmQueue(void);
static void UnlockAlarmQueue(void);
static void InitAlarmSystem(void);
static BOOL IsAlarmQueuedLocked(const OSAlarm* alarm);
static BOOL RemoveAlarmLocked(OSAlarm* alarm, BOOL clearHandler);

#ifndef _WIN32
static void AddMillisecondsToTimespec(struct timespec* ts, u32 ms);
#endif

/*---------------------------------------------------------------------------*
  Name:         LockAlarmQueue / UnlockAlarmQueue

  Description:  Thread-safe queue access. These simulate OSDisableInterrupts/
                OSRestoreInterrupts from the original implementation.
 *---------------------------------------------------------------------------*/
static void LockAlarmQueue(void) {
#ifdef _WIN32
    EnterCriticalSection(&s_alarmMutex);
#else
    pthread_mutex_lock(&s_alarmMutex);
#endif
}

static void UnlockAlarmQueue(void) {
#ifdef _WIN32
    LeaveCriticalSection(&s_alarmMutex);
#else
    pthread_mutex_unlock(&s_alarmMutex);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         InitAlarmSystem

  Description:  Initializes the alarm system - creates the background timer
                thread that processes alarm callbacks.
 *---------------------------------------------------------------------------*/
static void InitAlarmSystem(void) {
    if (s_alarmSystemInitialized) {
        return;
    }
    
#ifdef _WIN32
    InitializeCriticalSection(&s_alarmMutex);
    s_alarmEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    s_alarmThreadRunning = TRUE;
    s_alarmThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AlarmThreadFunc, 
                                 NULL, 0, NULL);
#else
    pthread_mutex_init(&s_alarmMutex, NULL);
    pthread_cond_init(&s_alarmCond, NULL);
    s_alarmThreadRunning = TRUE;
    pthread_create(&s_alarmThread, NULL, AlarmThreadFunc, NULL);
#endif
    
    s_alarmSystemInitialized = TRUE;
}

static BOOL IsAlarmQueuedLocked(const OSAlarm* alarm) {
    for (OSAlarm* current = s_alarmQueue.head; current; current = current->next) {
        if (current == alarm) {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL RemoveAlarmLocked(OSAlarm* alarm, BOOL clearHandler) {
    if (!alarm || !IsAlarmQueuedLocked(alarm)) {
        return FALSE;
    }

    const BOOL wasHead = (s_alarmQueue.head == alarm);

    if (alarm->next == NULL) {
        s_alarmQueue.tail = alarm->prev;
    } else {
        alarm->next->prev = alarm->prev;
    }

    if (alarm->prev) {
        alarm->prev->next = alarm->next;
    } else {
        s_alarmQueue.head = alarm->next;
    }

    alarm->prev = NULL;
    alarm->next = NULL;
    if (clearHandler) {
        alarm->handler = NULL;
    }

    if (wasHead) {
#ifdef _WIN32
        SetEvent(s_alarmEvent);
#else
        pthread_cond_signal(&s_alarmCond);
#endif
    }

    return TRUE;
}

#ifndef _WIN32
static void AddMillisecondsToTimespec(struct timespec* ts, u32 ms) {
    if (!ts) {
        return;
    }

    ts->tv_sec += (time_t)(ms / 1000);
    long nsec = ts->tv_nsec + (long)((ms % 1000) * 1000000L);
    if (nsec >= 1000000000L) {
        ts->tv_sec += 1;
        nsec -= 1000000000L;
    }
    ts->tv_nsec = nsec;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         AlarmThreadFunc

  Description:  Background thread that processes alarm callbacks. This simulates
                the Decrementer Exception Handler from the original hardware.
                
                It sleeps until the next alarm should fire, then calls the
                handler and reschedules periodic alarms.
 *---------------------------------------------------------------------------*/
static void* AlarmThreadFunc(void* arg) {
    (void)arg;
    
    while (s_alarmThreadRunning) {
        LockAlarmQueue();
        
        OSAlarm* alarm = s_alarmQueue.head;
        
        if (alarm == NULL) {
            // No alarms pending, wait for signal
#ifdef _WIN32
            UnlockAlarmQueue();
            WaitForSingleObject(s_alarmEvent, INFINITE);
#else
            while (s_alarmThreadRunning && s_alarmQueue.head == NULL) {
                pthread_cond_wait(&s_alarmCond, &s_alarmMutex);
            }
            UnlockAlarmQueue();
#endif
            continue;
        }
        
        // Check if alarm should fire
        OSTime currentTime = OSGetTime();
        
        if (currentTime < alarm->fire) {
            // Sleep until alarm fires
            OSTime sleepTime = alarm->fire - currentTime;
            // Convert to milliseconds for sleep (cap to avoid long blocks / overflow)
            u32 sleepMs = (u32)(sleepTime / (OS_TIMER_CLOCK / 1000));
            if (sleepMs == 0) sleepMs = 1;
            if (sleepMs > 5000) sleepMs = 5000;  /* wake every 5s to re-check; SetEvent can wake earlier */
            
#ifdef _WIN32
            UnlockAlarmQueue();
            WaitForSingleObject(s_alarmEvent, sleepMs);
#else
            struct timespec wakeTime;
            clock_gettime(CLOCK_REALTIME, &wakeTime);
            AddMillisecondsToTimespec(&wakeTime, sleepMs);
            pthread_cond_timedwait(&s_alarmCond, &s_alarmMutex, &wakeTime);
            UnlockAlarmQueue();
#endif
            continue;
        }
        
        // Alarm should fire! Capture metadata then remove from queue.
        OSAlarmHandler handler = alarm->handler;
        OSTime period = alarm->period;
        OSTime start = alarm->start;

        RemoveAlarmLocked(alarm, TRUE);
        
        // For periodic alarms, calculate next fire time and re-insert
        if (period > 0 && handler) {
            alarm->period = period; // Restore in case it was cleared
            alarm->start = start;
            InsertAlarm(alarm, 0, handler); // InsertAlarm handles periodic calculation
        }
        
        UnlockAlarmQueue();
        
        // Call the handler (outside the lock to avoid deadlock)
        // Note: Original hardware calls this with a fresh OSContext, but we
        // don't have that luxury on PC. Handler runs in timer thread context.
        if (handler) {
            handler(alarm, NULL);
        }
    }
    
    return NULL;
}

/*---------------------------------------------------------------------------*
  Name:         InsertAlarm

  Description:  Inserts an alarm into the queue in sorted order by fire time.
                This is called with interrupts disabled (mutex locked).
                
                For periodic alarms, calculates the next fire time based on
                the period and current time.

  Arguments:    alarm   - Alarm to insert
                fire    - Fire time (ignored for periodic, calculated instead)
                handler - Handler to call when alarm fires
 *---------------------------------------------------------------------------*/
static void InsertAlarm(OSAlarm* alarm, OSTime fire, OSAlarmHandler handler) {
    // Calculate next fire for periodic alarms
    if (alarm->period > 0) {
        OSTime currentTime = OSGetTime();
        fire = alarm->start;
        
        if (alarm->start < currentTime) {
            // We're past the start time, calculate next fire
            OSTime elapsed = currentTime - alarm->start;
            OSTime periods = elapsed / alarm->period + 1;
            fire = alarm->start + (periods * alarm->period);
        }
    }
    
    alarm->handler = handler;
    alarm->fire = fire;
    
    // Insert in sorted order (earliest fire time first)
    OSAlarm* next;
    OSAlarm* prev;
    
    for (next = s_alarmQueue.head; next != NULL; next = next->next) {
        if (next->fire > fire) {
            // Insert before 'next'
            alarm->prev = next->prev;
            alarm->next = next;
            next->prev = alarm;
            
            prev = alarm->prev;
            if (prev) {
                prev->next = alarm;
            } else {
                // Inserting at head
                s_alarmQueue.head = alarm;
            }
            
            // Wake up timer thread since queue changed
#ifdef _WIN32
            SetEvent(s_alarmEvent);
#else
            pthread_cond_signal(&s_alarmCond);
#endif
            return;
        }
    }
    
    // Insert at tail
    alarm->next = NULL;
    prev = s_alarmQueue.tail;
    s_alarmQueue.tail = alarm;
    alarm->prev = prev;
    
    if (prev) {
        prev->next = alarm;
    } else {
        // Queue was empty
        s_alarmQueue.head = s_alarmQueue.tail = alarm;
    }
    
    // Wake up timer thread
#ifdef _WIN32
    SetEvent(s_alarmEvent);
#else
    pthread_cond_signal(&s_alarmCond);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OSCreateAlarm

  Description:  Initializes an alarm structure. Must be called before using
                the alarm with OSSetAlarm, etc.

  Arguments:    alarm - Pointer to alarm structure to initialize

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSCreateAlarm(OSAlarm* alarm) {
    if (!alarm) return;
    
    memset(alarm, 0, sizeof(OSAlarm));
}

/*---------------------------------------------------------------------------*
  Name:         OSSetAlarm

  Description:  Sets a one-shot alarm to fire after 'tick' time units from now.
                This is a relative time alarm.

  Arguments:    alarm   - Pointer to alarm structure (must call OSCreateAlarm first)
                tick    - Time until alarm fires (in OS time units)
                handler - Function to call when alarm fires

  Returns:      None
  
  Notes:        On original hardware, the handler is called from interrupt
                context. On PC, it's called from a timer thread.
 *---------------------------------------------------------------------------*/
void OSSetAlarm(OSAlarm* alarm, OSTime tick, OSAlarmHandler handler) {
    if (!alarm || !handler) return;
    
    if (!s_alarmSystemInitialized) {
        InitAlarmSystem();
    }
    
    LockAlarmQueue();

    // Re-arming the same OSAlarm must replace any existing queued instance.
    RemoveAlarmLocked(alarm, TRUE);
    
    alarm->period = 0; // One-shot alarm
    alarm->start = 0;
    InsertAlarm(alarm, OSGetTime() + tick, handler);
    
    UnlockAlarmQueue();
}

/*---------------------------------------------------------------------------*
  Name:         OSSetAbsAlarm

  Description:  Sets a one-shot alarm to fire at an absolute time.

  Arguments:    alarm   - Pointer to alarm structure
                time    - Absolute time when alarm should fire
                handler - Function to call when alarm fires

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSSetAbsAlarm(OSAlarm* alarm, OSTime time, OSAlarmHandler handler) {
    if (!alarm || !handler) return;
    
    if (!s_alarmSystemInitialized) {
        InitAlarmSystem();
    }
    
    LockAlarmQueue();

    // Re-arming the same OSAlarm must replace any existing queued instance.
    RemoveAlarmLocked(alarm, TRUE);
    
    alarm->period = 0; // One-shot alarm
    alarm->start = 0;
    InsertAlarm(alarm, time, handler);
    
    UnlockAlarmQueue();
}

/*---------------------------------------------------------------------------*
  Name:         OSSetPeriodicAlarm

  Description:  Sets a periodic alarm that fires repeatedly. After each fire,
                the alarm is automatically rescheduled for the next period.

  Arguments:    alarm   - Pointer to alarm structure
                start   - Absolute time for first fire
                period  - Time between fires
                handler - Function to call each time alarm fires

  Returns:      None
  
  Notes:        The alarm will keep firing until OSCancelAlarm is called.
 *---------------------------------------------------------------------------*/
void OSSetPeriodicAlarm(OSAlarm* alarm, OSTime start, OSTime period, 
                       OSAlarmHandler handler) {
    if (!alarm || !handler || period <= 0) return;
    
    if (!s_alarmSystemInitialized) {
        InitAlarmSystem();
    }
    
    LockAlarmQueue();

    // Re-arming the same OSAlarm must replace any existing queued instance.
    RemoveAlarmLocked(alarm, TRUE);
    
    alarm->period = period;
    alarm->start = start;
    InsertAlarm(alarm, 0, handler); // InsertAlarm calculates fire time
    
    UnlockAlarmQueue();
}

/*---------------------------------------------------------------------------*
  Name:         OSCancelAlarm

  Description:  Cancels a pending alarm. If the alarm is not in the queue,
                this does nothing.

  Arguments:    alarm - Pointer to alarm to cancel

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSCancelAlarm(OSAlarm* alarm) {
    if (!alarm) return;
    
    LockAlarmQueue();

    if (!RemoveAlarmLocked(alarm, TRUE)) {
        // Not in queue: ensure links are sanitized anyway.
        alarm->handler = NULL;
        alarm->prev = NULL;
        alarm->next = NULL;
    }

    UnlockAlarmQueue();
}

/*---------------------------------------------------------------------------*
  Name:         OSCancelAlarms

  Description:  Cancels all alarms with a specific tag. Tags are used to
                group related alarms for batch cancellation.

  Arguments:    tag - Tag value to match (must be non-zero)

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSCancelAlarms(u32 tag) {
    if (tag == 0) return; // Tag 0 is reserved
    
    LockAlarmQueue();
    
    OSAlarm* alarm = s_alarmQueue.head;
    while (alarm) {
        OSAlarm* next = alarm->next;
        if (alarm->tag == tag) {
            // Unlock before calling OSCancelAlarm since it locks
            UnlockAlarmQueue();
            OSCancelAlarm(alarm);
            LockAlarmQueue();
        }
        alarm = next;
    }
    
    UnlockAlarmQueue();
}

/*---------------------------------------------------------------------------*
  Name:         OSCheckAlarmQueue

  Description:  Debug function to validate alarm queue integrity.

  Arguments:    None

  Returns:      TRUE if queue is valid, FALSE otherwise
 *---------------------------------------------------------------------------*/
BOOL OSCheckAlarmQueue(void) {
    LockAlarmQueue();
    
    // Check head/tail consistency
    if ((s_alarmQueue.head == NULL && s_alarmQueue.tail != NULL) ||
        (s_alarmQueue.head != NULL && s_alarmQueue.tail == NULL)) {
        UnlockAlarmQueue();
        return FALSE;
    }
    
    // Check head has no prev
    if (s_alarmQueue.head && s_alarmQueue.head->prev != NULL) {
        UnlockAlarmQueue();
        return FALSE;
    }
    
    // Check tail has no next
    if (s_alarmQueue.tail && s_alarmQueue.tail->next != NULL) {
        UnlockAlarmQueue();
        return FALSE;
    }
    
    // Walk the list and verify linkage
    for (OSAlarm* alarm = s_alarmQueue.head; alarm; alarm = alarm->next) {
        // Check next->prev points back to us
        if (alarm->next && alarm->next->prev != alarm) {
            UnlockAlarmQueue();
            return FALSE;
        }
        
        // Check we're at tail if we have no next
        if (alarm->next == NULL && s_alarmQueue.tail != alarm) {
            UnlockAlarmQueue();
            return FALSE;
        }
    }
    
    UnlockAlarmQueue();
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         OSSetAlarmTag

  Description:  Sets a tag on an alarm. Tags are used to group alarms for
                batch operations like OSCancelAlarms.

  Arguments:    alarm - Pointer to alarm
                tag   - Tag value (0 is reserved, don't use)

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSSetAlarmTag(OSAlarm* alarm, u32 tag) {
    if (alarm) {
        alarm->tag = tag;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OSSetAlarmUserData / OSGetAlarmUserData

  Description:  Stores/retrieves arbitrary user data with an alarm.

  Arguments:    alarm - Pointer to alarm
                data  - User data pointer

  Returns:      User data pointer (Get function)
 *---------------------------------------------------------------------------*/
void OSSetAlarmUserData(OSAlarm* alarm, void* data) {
    if (alarm) {
        alarm->userData = data;
    }
}

void* OSGetAlarmUserData(const OSAlarm* alarm) {
    return alarm ? alarm->userData : NULL;
}

/*---------------------------------------------------------------------------*
  Name:         OSInitAlarm

  Description:  Initialize the alarm subsystem. Called internally during
                OSInit(). Exposed for modules that need to verify alarm
                system is ready (e.g., CARD module).
                
                On GC/Wii: Sets up alarm lists and interrupt handlers
                On PC: Already initialized in OSInit(), this is a no-op stub

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSInitAlarm(void) {
    if (!s_alarmSystemInitialized) {
        InitAlarmSystem();
    }
}
