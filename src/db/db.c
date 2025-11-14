/*---------------------------------------------------------------------------*
  db.c - Dolphin Debugger Interface for libPorpoise
  
  PC-compatible stubs for the Dolphin Debugger interface.
  On PC, debugger functions output to console instead of serial debugger.
 *---------------------------------------------------------------------------*/

#include <dolphin/types.h>
#include <dolphin/os.h>
#include <dolphin/db.h>
#include <string.h>

/* Global debugger interface */
DBInterface* __DBInterface = NULL;

/* Verbose flag - controls debug output verbosity */
BOOL DBVerbose = FALSE;

/* Static storage for debugger interface (since we don't have low memory) */
static DBInterface s_dbInterface;

/*---------------------------------------------------------------------------*
  Name:         DBInit
  
  Description:  Initialize the debugger interface.
                On PC, this just sets up the interface structure.
  
  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DBInit(void) {
    /* Initialize the debugger interface */
    __DBInterface = &s_dbInterface;
    memset(__DBInterface, 0, sizeof(DBInterface));
    
    /* On PC, no debugger is present */
    __DBInterface->bPresent = 0;
    __DBInterface->exceptionMask = 0;
    __DBInterface->ExceptionDestination = __DBExceptionDestination;
    __DBInterface->exceptionReturn = NULL;
    
    /* Enable verbose output by default on PC for easier debugging */
    DBVerbose = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         DBIsDebuggerPresent
  
  Description:  Check if the debugger is present.
                On PC, always returns FALSE.
  
  Arguments:    None.
  
  Returns:      TRUE if debugger present, FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL DBIsDebuggerPresent(void) {
    if (__DBInterface == NULL) {
        return FALSE;
    }
    return (BOOL)__DBInterface->bPresent;
}

/*---------------------------------------------------------------------------*
  Name:         __DBIsExceptionMarked
  
  Description:  Check if an exception is marked for debugger handling.
  
  Arguments:    exception - Exception number
  
  Returns:      TRUE if marked, FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL __DBIsExceptionMarked(int exception) {
    if (__DBInterface == NULL) {
        return FALSE;
    }
    
    u32 mask = (u32)(1 << exception);
    return (BOOL)(__DBInterface->exceptionMask & mask);
}

/*---------------------------------------------------------------------------*
  Name:         __DBExceptionDestination
  
  Description:  Exception destination called from exception handlers.
                On PC, this dumps context and halts.
  
  Arguments:    None.
  
  Returns:      None (does not return).
 *---------------------------------------------------------------------------*/
void __DBExceptionDestination(void) {
    OSReport("========================================\n");
    OSReport("DBExceptionDestination called\n");
    OSReport("Exception occurred - dumping context\n");
    OSReport("========================================\n");
    
    /* On PC, we can't dump the actual CPU context like on PowerPC,
     * but we can at least report that an exception occurred */
    OSReport("Note: Full context dump not available on PC\n");
    OSReport("This would normally show PowerPC register state\n");
    OSReport("========================================\n");
    
    /* Halt execution */
    OSPanic(__FILE__, __LINE__, "Debugger exception destination reached");
}

/*---------------------------------------------------------------------------*
  Name:         __DBSetPresent
  
  Description:  Testing function to set debugger present flag.
                Debug builds only.
  
  Arguments:    value - TRUE to mark debugger as present
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef _DEBUG
void __DBSetPresent(u32 value) {
    if (__DBInterface == NULL) {
        DBInit();
    }
    __DBInterface->bPresent = value;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         __DBMarkException
  
  Description:  Testing function to mark an exception for debugger handling.
                Debug builds only.
  
  Arguments:    exception - Exception number
                value     - TRUE to mark, FALSE to unmark
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef _DEBUG
void __DBMarkException(int exception, BOOL value) {
    if (__DBInterface == NULL) {
        DBInit();
    }
    
    u32 mask = (u32)(1 << exception);
    
    if (value) {
        __DBInterface->exceptionMask |= mask;
    } else {
        __DBInterface->exceptionMask &= ~mask;
    }
}
#endif

