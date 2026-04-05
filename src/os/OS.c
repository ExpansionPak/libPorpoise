#include <dolphin/os.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#if defined(_MSC_VER)
#define OS_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
#define OS_THREAD_LOCAL __thread
#else
#define OS_THREAD_LOCAL _Thread_local
#endif

static BOOL s_osInitialized = FALSE;
static BOOL s_osReportInitialized = FALSE;
/* Per-thread guard against recursive OSReport calls. */
static OS_THREAD_LOCAL BOOL s_osReportInProgress = FALSE;
#ifdef _WIN32
static BOOL s_consoleAttached = FALSE;
/* Optional OSReport file sink enabled via PORPOISE_OSREPORT_FILE env var. */
static int s_osReportFileMode = -1; /* -1 unknown, 0 disabled, 1 enabled */
static char s_osReportFilePath[1024] = {0};

static void EnsureConsole(void) {
    if (s_consoleAttached) {
        return;
    }

    if (!GetConsoleWindow()) {
        if (!AllocConsole()) {
            AttachConsole(ATTACH_PARENT_PROCESS);
        }
    }

    FILE* out = NULL;
    if (freopen_s(&out, "CONOUT$", "w", stdout) == 0) {
        setvbuf(stdout, NULL, _IONBF, 0);
    }
    if (freopen_s(&out, "CONOUT$", "w", stderr) == 0) {
        setvbuf(stderr, NULL, _IONBF, 0);
    }

    s_consoleAttached = TRUE;
}

static void WriteOSReportToFile(const char* text, size_t len) {
    if (!text || len == 0) {
        return;
    }

    if (s_osReportFileMode < 0) {
        DWORD got = GetEnvironmentVariableA("PORPOISE_OSREPORT_FILE",
                                            s_osReportFilePath,
                                            (DWORD)sizeof(s_osReportFilePath));
        if (got == 0 || got >= sizeof(s_osReportFilePath)) {
            /* Default log path for local debugging when env var is not set. */
            strncpy_s(s_osReportFilePath,
                      sizeof(s_osReportFilePath),
                      "osreport_runtime.log",
                      _TRUNCATE);
            s_osReportFileMode = 1;
        } else {
            s_osReportFileMode = 1;
        }
    }

    if (s_osReportFileMode != 1) {
        return;
    }

    FILE* f = NULL;
    if (fopen_s(&f, s_osReportFilePath, "ab") != 0 || !f) {
        return;
    }

    fwrite(text, 1, len, f);
    fclose(f);
}
#endif

/* Arena management variables
 * On GC/Wii, "arenas" define ranges of memory available for allocation
 * MEM1 = 24MB main RAM, MEM2 = 64MB external RAM (Wii only)
 * On PC, we don't pre-allocate arenas - games can use malloc directly
 */
static void* s_arenaHi = NULL;
static void* s_arenaLo = NULL;
static void* s_mem1ArenaHi = NULL;
static void* s_mem1ArenaLo = NULL;
static void* s_mem2ArenaHi = NULL;
static void* s_mem2ArenaLo = NULL;

static u32 NormalizeArenaAlign(u32 align) {
    return (align == 0) ? 4u : align;
}

static uintptr_t AlignUpAddress(uintptr_t value, u32 align) {
    uintptr_t a = (uintptr_t)align;
    uintptr_t rem = value % a;
    if (rem == 0) {
        return value;
    }
    return value + (a - rem);
}

static uintptr_t AlignDownAddress(uintptr_t value, u32 align) {
    uintptr_t a = (uintptr_t)align;
    return value - (value % a);
}

/*---------------------------------------------------------------------------*
  Name:         OSInit

  Description:  Initializes the operating system. This is the first OS
                function that should be called. On the original hardware,
                this sets up exception handlers, interrupts, caches, and
                various hardware subsystems.
                
                On PC, we simulate the memory layout and initialize our
                platform-specific implementations of threading, timing, etc.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void __OSThreadInit(void);

void OSInit(void) {
    if (s_osInitialized) {
        return;
    }
    
    s_osInitialized = TRUE;

#ifdef _WIN32
    EnsureConsole();
#endif
    
    __OSThreadInit();
    
    // On PC, we don't pre-allocate memory arenas like the original hardware
    // Games can use malloc directly, or call OSInitAlloc() with their own ranges
    // Arena pointers are initialized to NULL - games set them if needed
    
    s_arenaLo = NULL;
    s_arenaHi = NULL;
    s_mem1ArenaLo = NULL;
    s_mem1ArenaHi = NULL;
    s_mem2ArenaLo = NULL;
    s_mem2ArenaHi = NULL;
    
    // Report initialization
    OSReport("==================================\n");
    OSReport("libPorpoise v0.1.0\n");
    OSReport("GC/Wii SDK PC Port\n");
    OSReport("==================================\n");
    OSReport("Initialized: %s %s\n", __DATE__, __TIME__);
    OSReport("Memory: Using standard malloc/free (no pre-allocated arenas)\n");
    OSReport("==================================\n");
}

/*---------------------------------------------------------------------------*
  Name:         OSGetConsoleType

  Description:  Returns the console type identifier. On original hardware,
                this returns values like OS_CONSOLE_RVL_RETAIL1 (retail Wii),
                OS_CONSOLE_RVL_NDEV2_0 (dev kit), etc.
                
                On PC, we return a custom identifier to indicate this is a
                PC port. Games may check this value to enable/disable features
                or adjust behavior.

  Arguments:    None.

  Returns:      Console type identifier. 0x10000000 = PC port
 *---------------------------------------------------------------------------*/
#define OS_CONSOLE_PC_PORT 0x10000000

u32 OSGetConsoleType(void) {
    return OS_CONSOLE_PC_PORT;
}

/*---------------------------------------------------------------------------*
  Name:         OSReport

  Description:  Debug output function similar to printf. On retail hardware
                this typically does nothing, but on dev kits it outputs to
                the debugger or USB Gecko device.
                
                On PC, we simply output to stdout. This is used extensively
                by games for debug logging.

  Arguments:    fmt  - printf-style format string
                ...  - variable arguments

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSReport(const char* fmt, ...) {
    if (s_osReportInProgress) {
        /* Prevent infinite recursion if OSReport is triggered while already running */
#ifdef _WIN32
        OutputDebugStringA("[OSReport] Recursion detected!\n");
#else
        fputs("[OSReport] Recursion detected!\n", stderr);
        fflush(stderr);
#endif
        return;
    }

    if (!fmt) {
#ifdef _WIN32
        OutputDebugStringA("[OSReport] NULL format string\n");
#else
        fputs("[OSReport] NULL format string\n", stderr);
        fflush(stderr);
#endif
        return;
    }

    s_osReportInProgress = TRUE;

    if (!s_osReportInitialized) {
        s_osReportInitialized = TRUE;
    }

    char buffer[2048];
    size_t offset = 0;

    /* Optional timestamp for easier debugging */
    if (s_osInitialized) {
        time_t now = time(NULL);
        if (now != (time_t)-1) {
            struct tm* tm_info = localtime(&now);
            if (tm_info) {
                offset = (size_t)snprintf(buffer, sizeof(buffer), "[%02d:%02d:%02d] ",
                                          tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
                if (offset >= sizeof(buffer)) {
                    offset = sizeof(buffer) - 1;
                }
            }
        }
    }

    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buffer + offset, sizeof(buffer) - offset, fmt, args);
    va_end(args);

    if (written < 0) {
#ifdef _WIN32
        OutputDebugStringA("[OSReport] Formatting error\n");
#else
        fputs("[OSReport] Formatting error\n", stderr);
        fflush(stderr);
#endif
        s_osReportInProgress = FALSE;
        return;
    }

    size_t totalLen = offset + (size_t)written;
    if (totalLen >= sizeof(buffer)) {
        totalLen = sizeof(buffer) - 1;
    }

#ifdef _WIN32
    buffer[totalLen] = '\0';
    /* Use Visual Studio debug output only - avoids console deadlock issues */
    OutputDebugStringA(buffer);
    WriteOSReportToFile(buffer, totalLen);
#else
    fwrite(buffer, 1, totalLen, stdout);
    fflush(stdout);
#endif

    s_osReportInProgress = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OSPanic

  Description:  Fatal error handler. This is called when the game encounters
                an unrecoverable error. On original hardware, this displays
                an error screen and halts execution.
                
                On PC, we print the error and abort the program.

  Arguments:    file - Source file where panic occurred
                line - Line number where panic occurred  
                fmt  - printf-style format string
                ...  - variable arguments

  Returns:      Does not return (aborts program).
 *---------------------------------------------------------------------------*/
void OSPanic(const char* file, int line, const char* fmt, ...) {
    va_list args;
    char buf[256];
    int is_graceful = 0;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* Treat "End of test/demo/program" as normal shutdown - exit cleanly */
    if (strstr(buf, "End of test") || strstr(buf, "End of demo") ||
        strstr(buf, "End of program") || strstr(buf, "End of demo\n")) {
        is_graceful = 1;
    }

    fprintf(stderr, "========================================\n");
    fprintf(stderr, "%s\n", is_graceful ? "         Demo Ended" : "         PANIC - FATAL ERROR");
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "Location: %s:%d\n", file, line);
    fprintf(stderr, "Message:  %s\n", buf);
    fprintf(stderr, "========================================\n");
    fflush(stderr);

    if (is_graceful) {
        exit(0);
    }
    abort();
}

/*---------------------------------------------------------------------------*
  Arena Management Functions
  
  These functions manage memory arenas - contiguous ranges of memory that
  games can subdivide and allocate from. The original hardware has:
  - MEM1: 24MB main RAM (NAPA)
  - MEM2: 64MB external RAM (GDDR3, Wii only)
  
  Games typically get the arena bounds and set up their own allocators
  within those ranges. We simulate this by allocating large blocks from
  the system heap.
 *---------------------------------------------------------------------------*/

/* Default arena */
void* OSGetArenaHi(void) { 
    return s_arenaHi; 
}
void* OSGetArenaLo(void) { 
    return s_arenaLo; 
}
void  OSSetArenaHi(void* addr) { 
    s_arenaHi = addr; 
}
void  OSSetArenaLo(void* addr) { 
    s_arenaLo = addr; 
}

void* OSAllocFromArenaLo(u32 size, u32 align) {
    u32 normalizedAlign = NormalizeArenaAlign(align);

    if (!s_arenaLo || !s_arenaHi) {
        return NULL;
    }

    uintptr_t lo = (uintptr_t)s_arenaLo;
    uintptr_t hi = (uintptr_t)s_arenaHi;
    if (lo > hi) {
        return NULL;
    }

    uintptr_t start = AlignUpAddress(lo, normalizedAlign);
    if (start < lo) {
        return NULL;
    }

    if ((uintptr_t)size > (hi - start)) {
        return NULL;
    }

    uintptr_t newLo = start + (uintptr_t)size;
    if (newLo < start || newLo > hi) {
        return NULL;
    }

    s_arenaLo = (void*)newLo;
    return (void*)start;
}

void* OSAllocFromArenaHi(u32 size, u32 align) {
    u32 normalizedAlign = NormalizeArenaAlign(align);

    if (!s_arenaLo || !s_arenaHi) {
        return NULL;
    }

    uintptr_t lo = (uintptr_t)s_arenaLo;
    uintptr_t hi = (uintptr_t)s_arenaHi;
    if (lo > hi || (uintptr_t)size > (hi - lo)) {
        return NULL;
    }

    uintptr_t candidate = hi - (uintptr_t)size;
    uintptr_t newHi = AlignDownAddress(candidate, normalizedAlign);
    if (newHi > hi || newHi < lo) {
        return NULL;
    }

    s_arenaHi = (void*)newHi;
    return (void*)newHi;
}

/* MEM1 arena (24MB main RAM) - stubs for compatibility */
void* OSGetMEM1ArenaHi(void) { return s_mem1ArenaHi; }
void* OSGetMEM1ArenaLo(void) { return s_mem1ArenaLo; }
void  OSSetMEM1ArenaHi(void* addr) { s_mem1ArenaHi = addr; }
void  OSSetMEM1ArenaLo(void* addr) { s_mem1ArenaLo = addr; }

/* MEM2 arena (64MB external RAM, Wii only) - stubs for compatibility */
void* OSGetMEM2ArenaHi(void) { return s_mem2ArenaHi; }
void* OSGetMEM2ArenaLo(void) { return s_mem2ArenaLo; }
void  OSSetMEM2ArenaHi(void* addr) { s_mem2ArenaHi = addr; }
void  OSSetMEM2ArenaLo(void* addr) { s_mem2ArenaLo = addr; }

/*---------------------------------------------------------------------------*
  Name:         __OSGetDIConfig

  Description:  Get DVD Interface (DI) configuration. Used by DVD module
                to determine disc drive capabilities and settings.
                
                On GC/Wii: Reads DI configuration register
                On PC: Returns fake config (no DVD hardware)

  Arguments:    None

  Returns:      DI configuration value (always 0xFF on PC = no hardware)
 *---------------------------------------------------------------------------*/
u8 __OSGetDIConfig(void) {
    /* On PC, there's no DVD interface hardware.
     * Return 0xFF to indicate no DI hardware present.
     * 
     * On original hardware, this returns:
     * - Bit 7: 1 = DI present
     * - Bits 6-0: Configuration flags
     */
    return 0xFF;
}

/*---------------------------------------------------------------------------*
  Name:         __OSPSInit

  Description:  Initialize processor state. Sets up PowerPC-specific
                processor features like HID registers, exception vectors, etc.
                
                On GC/Wii: Configures HID0/HID2, sets up BAT registers, etc.
                On PC: No-op stub (no PowerPC processor state to initialize)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void __OSPSInit(void) {
    /* On PC, there's no PowerPC processor state to initialize.
     * 
     * On original hardware, this function:
     * - Sets up HID0 (Hardware Implementation Dependent register 0)
     * - Enables instruction/data caches
     * - Configures write gathering, speculative loading
     * - Sets up BAT (Block Address Translation) registers
     * - Enables/disables various CPU features
     * 
     * These are all PowerPC-specific and don't apply to x86/x64.
     */
}

/*---------------------------------------------------------------------------*
  Name:         __OSCacheInit

  Description:  Initialize the cache subsystem. Sets up L1/L2 cache
                parameters and locking.
                
                On GC/Wii: Configures L1 instruction/data cache (32KB each)
                           and L2 cache (256KB)
                On PC: No-op stub (cache already initialized in OSInit)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void __OSCacheInit(void) {
    /* Cache subsystem already initialized in OSInit().
     * 
     * On original hardware, this function:
     * - Invalidates all cache lines
     * - Sets up cache locking registers
     * - Configures L2 cache mode (data only, instruction+data, etc.)
     * 
     * On PC, our cache emulation (if enabled) is set up in OSInit()
     * via GeckoMemoryInit(). This stub exists for API compatibility.
     */
}

/*---------------------------------------------------------------------------*
  Name:         OSRegisterVersion

  Description:  Register a library version string. Used by SDK modules to
                log their version at initialization for debugging.
                
                On GC/Wii: Stores version strings in internal list
                On PC: Just logs to console

  Arguments:    id  Version string (usually from DOLPHIN_LIB_VERSION macro)

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSRegisterVersion(const char* id) {
    if (id) {
        OSReport("Library version registered: %s\n", id);
    }
}

/*---------------------------------------------------------------------------*
  Name:         OSFatal

  Description:  Display fatal error message and halt. Similar to OSPanic but
                with on-screen display capability.
                
                On GC/Wii: Renders error screen with colored background
                On PC: Logs error and calls OSPanic

  Arguments:    textColor   Foreground color (unused on PC)
                bgColor     Background color (unused on PC)
                msg         Error message

  Returns:      Does not return
 *---------------------------------------------------------------------------*/
void OSFatal(u32 textColor, u32 bgColor, const char* msg) {
    (void)textColor;
    (void)bgColor;
    
    OSReport("==================================================\n");
    OSReport("FATAL ERROR\n");
    OSReport("==================================================\n");
    OSReport("%s\n", msg);
    OSReport("==================================================\n");
    
    OSPanic(__FILE__, __LINE__, "OSFatal: %s", msg);
}
