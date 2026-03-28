/*---------------------------------------------------------------------------*
  OSMutex.c - Mutex and Condition Variable Implementation
  
  Moved from OSThread.c to match original SDK structure.
 *---------------------------------------------------------------------------*/

#include <dolphin/os.h>

static void LinkMutexToThreadUnlocked(OSMutex* mutex, OSThread* thread) {
    if (!mutex || !thread) {
        return;
    }

    if (mutex->link.prev || mutex->link.next || thread->queueMutex.head == mutex) {
        return;
    }

    mutex->link.prev = thread->queueMutex.tail;
    mutex->link.next = NULL;
    if (thread->queueMutex.tail) {
        thread->queueMutex.tail->link.next = mutex;
    } else {
        thread->queueMutex.head = mutex;
    }
    thread->queueMutex.tail = mutex;
}

static void UnlinkMutexFromThreadUnlocked(OSMutex* mutex, OSThread* thread) {
    if (!mutex || !thread) {
        return;
    }

    if (mutex->link.prev) {
        mutex->link.prev->link.next = mutex->link.next;
    } else if (thread->queueMutex.head == mutex) {
        thread->queueMutex.head = mutex->link.next;
    }

    if (mutex->link.next) {
        mutex->link.next->link.prev = mutex->link.prev;
    } else if (thread->queueMutex.tail == mutex) {
        thread->queueMutex.tail = mutex->link.prev;
    }

    mutex->link.prev = NULL;
    mutex->link.next = NULL;
}

/*===========================================================================*
  MUTEX IMPLEMENTATION
 *===========================================================================*/

void OSInitMutex(OSMutex* mutex) {
    if (!mutex) return;
    
    mutex->queue.head = NULL;
    mutex->queue.tail = NULL;
    mutex->thread = NULL;
    mutex->count = 0;
    mutex->link.next = NULL;
    mutex->link.prev = NULL;
}

void OSLockMutex(OSMutex* mutex) {
    if (!mutex) return;
    
    OSThread* current = OSGetCurrentThread();
    BOOL enabled = OSDisableInterrupts();
    if (!current) {
        OSRestoreInterrupts(enabled);
        return;
    }
    
    /* Recursive locking - same thread can lock multiple times */
    if (mutex->thread == current) {
        mutex->count++;
        OSRestoreInterrupts(enabled);
        return;
    }
    
    /* Lock is held by another thread - wait in priority order queue */
    while (mutex->thread != NULL) {
        if (mutex->thread->priority > current->priority) {
            __OSPromoteThread(mutex->thread, current->priority);
        }
        OSSleepThread(&mutex->queue);
    }
    
    /* Acquire lock */
    mutex->thread = current;
    mutex->count = 1;
    LinkMutexToThreadUnlocked(mutex, current);
    OSRestoreInterrupts(enabled);
}

void OSUnlockMutex(OSMutex* mutex) {
    if (!mutex) return;
    
    OSThread* current = OSGetCurrentThread();
    BOOL enabled = OSDisableInterrupts();
    
    /* Only owner can unlock */
    if (mutex->thread != current) {
        OSRestoreInterrupts(enabled);
        return;
    }
    
    /* Decrement recursion count */
    mutex->count--;
    if (mutex->count > 0) {
        OSRestoreInterrupts(enabled);
        return;  /* Still locked (recursive) */
    }
    
    /* Release lock */
    UnlinkMutexFromThreadUnlocked(mutex, current);
    mutex->thread = NULL;
    
    /* Wake all waiters; scheduler/priority chooses next owner */
    OSWakeupThread(&mutex->queue);
    OSRestoreInterrupts(enabled);
}

BOOL OSTryLockMutex(OSMutex* mutex) {
    if (!mutex) return FALSE;
    
    OSThread* current = OSGetCurrentThread();
    BOOL enabled = OSDisableInterrupts();
    if (!current) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }
    
    /* Recursive locking */
    if (mutex->thread == current) {
        mutex->count++;
        OSRestoreInterrupts(enabled);
        return TRUE;
    }
    
    /* Try to acquire - fail if held */
    if (mutex->thread != NULL) {
        OSRestoreInterrupts(enabled);
        return FALSE;
    }
    
    mutex->thread = current;
    mutex->count = 1;
    LinkMutexToThreadUnlocked(mutex, current);
    OSRestoreInterrupts(enabled);
    return TRUE;
}

/*===========================================================================*
  CONDITION VARIABLE IMPLEMENTATION
 *===========================================================================*/

void OSInitCond(OSCond* cond) {
    if (!cond) return;
    cond->queue.head = NULL;
    cond->queue.tail = NULL;
}

void OSWaitCond(OSCond* cond, OSMutex* mutex) {
    if (!cond || !mutex) return;

    OSThread* current = OSGetCurrentThread();
    BOOL enabled = OSDisableInterrupts();
    if (!current) {
        OSRestoreInterrupts(enabled);
        return;
    }

    if (mutex->thread != current || mutex->count <= 0) {
        OSRestoreInterrupts(enabled);
        return;
    }

    /* SDK semantics: release mutex fully regardless of recursion depth,
     * then restore that depth after reacquiring.
     */
    const s32 savedCount = mutex->count;

    UnlinkMutexFromThreadUnlocked(mutex, current);
    mutex->count = 0;
    mutex->thread = NULL;
    OSWakeupThread(&mutex->queue);

    OSSleepThread(&cond->queue);

    while (mutex->thread != NULL && mutex->thread != current) {
        OSSleepThread(&mutex->queue);
    }

    if (mutex->thread != current) {
        mutex->thread = current;
        LinkMutexToThreadUnlocked(mutex, current);
    }
    mutex->count = savedCount;

    OSRestoreInterrupts(enabled);
}

void OSSignalCond(OSCond* cond) {
    if (!cond) return;

    /* Wake one waiting thread */
    OSWakeupThread(&cond->queue);
}

/*---------------------------------------------------------------------------*
  Name:         __OSUnlockAllMutex
  
  Description:  Unlocks all mutexes held by a thread. Used when thread
                terminates to clean up resources.
  
  Arguments:    thread - Thread whose mutexes should be unlocked
  
  Returns:      None
 *---------------------------------------------------------------------------*/
void __OSUnlockAllMutex(OSThread* thread) {
    if (!thread) return;
    
    OSMutex* mutex;
    
    // Unlock all mutexes in the thread's queue
    while (thread->queueMutex.head) {
        mutex = thread->queueMutex.head;
        
        // Remove from thread's mutex queue
        thread->queueMutex.head = mutex->link.next;
        if (mutex->link.next) {
            mutex->link.next->link.prev = NULL;
        } else {
            thread->queueMutex.tail = NULL;
        }
        
        // Clear mutex ownership
        mutex->count = 0;
        mutex->thread = NULL;
        
        // Wake waiting threads
        OSWakeupThread(&mutex->queue);
    }
}

