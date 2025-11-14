/*---------------------------------------------------------------------------*
  __start.c - Program Entry Point for libPorpoise
  
  This file provides the startup entry point for GameCube/Wii games ported
  to PC using libPorpoise. It replaces the original PowerPC-specific startup
  code with PC-compatible initialization.
  
  ARCHITECTURAL DIFFERENCES: GC/Wii vs PC
  ========================================
  
  On GC/Wii (PowerPC EABI):
  -------------------------
  - Assembly code in __start() initializes PowerPC registers (r1=stack, r2/r13=SDA)
  - Calls __init_hardware() to initialize hardware (cache, FPU, etc.)
  - Copies ROM data to RAM, initializes BSS sections
  - Calls OSInit() to initialize Dolphin OS
  - Calls __init_user() to run C++ static constructors
  - Calls main() with argc/argv
  - Calls exit() when done
  
  On PC (libPorpoise):
  --------------------
  - Standard C entry point (no assembly needed for register init)
  - Calls __init_hardware() for PC-specific initialization (stub)
  - No ROM/RAM copying needed (all code/data in memory)
  - BSS initialization handled by linker/OS
  - Calls OSInit() to initialize libPorpoise
  - Calls __init_user() to run C++ static constructors
  - Calls main() with argc/argv from OS
  - Calls exit() when done
  
  WHAT WE PRESERVE:
  - Same initialization sequence (hardware → data → OS → user → main)
  - C++ static constructor support
  - exit() and abort() implementations
  - Compatibility with games expecting this startup sequence
  
  WHAT'S DIFFERENT:
  - No PowerPC register initialization (x86/ARM handled by OS)
  - No ROM-to-RAM copying (all in memory on PC)
  - No hardware initialization (no GameCube/Wii hardware)
  - Standard C main() signature (matches original)
  
  USAGE:
  ------
  Games can define their own main() function. This __start() will be
  called first (if linked with libPorpoise init files).
  
  If games define their own __start(), it should call OSInit() before
  calling main() to ensure libPorpoise is initialized.
 *---------------------------------------------------------------------------*/

#include <dolphin/os.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    External Declarations
 *---------------------------------------------------------------------------*/

extern int main(int argc, char* argv[]);
extern void __init_hardware(void);
extern void __init_user(void);
extern void __flush_cache(void* address, unsigned int size);

/*---------------------------------------------------------------------------*
    Function Declarations
 *---------------------------------------------------------------------------*/

static void __init_data(void);

/*---------------------------------------------------------------------------*
  Name:         __start

  Description:  Main entry point for GameCube/Wii games on PC.
                Initializes libPorpoise and calls main().

  Arguments:    None (called by OS/C runtime)

  Returns:      Never returns (calls exit())
 *---------------------------------------------------------------------------*/
/* On PC, __start() can be used as an optional entry point.
 * If games define their own main(), they can either:
 * 1. Call OSInit() in their main() and not use __start()
 * 2. Use __start() which will call OSInit() and then their main()
 * 
 * Note: On PC, the standard entry point is main(), so __start() is
 * optional. Games can override it or just use standard main().
 */
#ifdef __GNUC__
__attribute__((weak))  /* Allow games to override */
#endif
void __start(void) {
    /* On PC, argc/argv are typically available from the OS.
     * For compatibility, we'll pass 0/NULL if not available.
     * Games can override __start() if they need proper argc/argv.
     */
    int argc = 0;
    char** argv = NULL;
    
#ifdef _WIN32
    /* On Windows, __argc and __argv are available */
    extern int __argc;
    extern char** __argv;
    if (__argc > 0) {
        argc = __argc;
        argv = __argv;
    }
#else
    /* On Unix-like systems, we'd need to parse /proc/self/cmdline or similar
     * For simplicity, we'll just pass 0/NULL and let games override if needed
     */
#endif

    /*
     * Hardware-level initialization (PC-specific stub)
     * On GC/Wii, this initializes cache, FPU, etc.
     * On PC, we just ensure libPorpoise is ready
     */
    __init_hardware();

    /*
     * Memory access is safe now.
     */

    /*
     * Data initialization: copy ROM data to RAM as necessary
     * On PC, all data is already in memory, but we call this
     * for compatibility with games that expect it
     */
    __init_data();

    /*
     * Initialize libPorpoise OS system
     * This sets up memory arenas, threading, timers, etc.
     */
    OSInit();

    /*
     * User-level initialization before main
     * This calls C++ static constructors if needed
     */
    __init_user();

    /*
     * Branch to main program
     */
    int exitCode = main(argc, argv);

    /*
     * Exit program
     */
    exit(exitCode);
    
    /* NOT REACHED */
}

/*---------------------------------------------------------------------------*
  Name:         __copy_rom_section

  Description:  Copy ROM section to RAM (stub on PC).
                On GC/Wii, this copies data from ROM to RAM.
                On PC, all data is already in memory.

  Arguments:    dst   Destination address
                src   Source address
                size  Size in bytes

  Returns:      None
 *---------------------------------------------------------------------------*/
static void __copy_rom_section(void* dst, const void* src, unsigned long size) {
    if (size && (dst != src)) {
        memcpy(dst, src, size);
        /* On PC, we don't need to flush cache (no instruction cache to invalidate) */
        /* But we call __flush_cache for compatibility */
        __flush_cache(dst, size);
    }
}

/*---------------------------------------------------------------------------*
  Name:         __init_bss_section

  Description:  Initialize BSS section to zeros (stub on PC).
                On GC/Wii, this zeros uninitialized data.
                On PC, linker/OS handles this automatically.

  Arguments:    dst   Destination address
                size  Size in bytes

  Returns:      None
 *---------------------------------------------------------------------------*/
static void __init_bss_section(void* dst, unsigned long size) {
    if (size) {
        memset(dst, 0, size);
    }
}

/*---------------------------------------------------------------------------*
  Name:         __init_data

  Description:  Initialize all data sections (stub on PC).
                On GC/Wii, this copies ROM data to RAM and zeros BSS.
                On PC, linker/OS handles this, but we call it for compatibility.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void __init_data(void) {
    /* On PC, linker handles .data and .bss initialization automatically */
    /* We provide this function for compatibility with games that expect it */
    
    /* Note: If games use custom linker scripts with _rom_copy_info or
     * _bss_init_info, they would be handled here. On PC, these are typically
     * not needed since all data is in memory.
     */
}

#ifdef __cplusplus
}
#endif

