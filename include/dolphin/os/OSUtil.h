#ifndef DOLPHIN_OS_UTIL_H
#define DOLPHIN_OS_UTIL_H

#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AT_ADDRESS
#if defined(__MWERKS__)
#define AT_ADDRESS(addr) : (addr)
#else
#define AT_ADDRESS(addr)
#endif
#endif

#ifndef OS_BASE_CACHED
#define OS_BASE_CACHED   0x80000000u
#endif

#ifndef OS_BASE_UNCACHED
#define OS_BASE_UNCACHED 0xC0000000u
#endif

#ifndef OS_CACHED_REGION_PREFIX
#define OS_CACHED_REGION_PREFIX   0x8000
#endif

#ifndef OS_UNCACHED_REGION_PREFIX
#define OS_UNCACHED_REGION_PREFIX 0xC000
#endif

#ifndef OS_PHYSICAL_MASK
#define OS_PHYSICAL_MASK          0x3FFF
#endif

#ifndef OSRoundUp32B
#define OSRoundUp32B(x)   (((u32)(x) + 0x1F) & ~(0x1F))
#endif

#ifndef OSRoundDown32B
#define OSRoundDown32B(x) (((u32)(x)) & ~(0x1F))
#endif

#ifndef OSPhysicalToCached
#define OSPhysicalToCached(paddr)    ((void*)((u32)(paddr) + OS_BASE_CACHED))
#endif

#ifndef OSPhysicalToUncached
#define OSPhysicalToUncached(paddr)  ((void*)((u32)(paddr) + OS_BASE_UNCACHED))
#endif

#ifndef OSCachedToPhysical
#define OSCachedToPhysical(caddr)    ((u32)((u8*)(caddr) - OS_BASE_CACHED))
#endif

#ifndef OSUncachedToPhysical
#define OSUncachedToPhysical(ucaddr) ((u32)((u8*)(ucaddr) - OS_BASE_UNCACHED))
#endif

#ifndef OSCachedToUncached
#define OSCachedToUncached(caddr)    ((void*)((u8*)(caddr) + (OS_BASE_UNCACHED - OS_BASE_CACHED)))
#endif

#ifndef OSUncachedToCached
#define OSUncachedToCached(ucaddr)   ((void*)((u8*)(ucaddr) - (OS_BASE_UNCACHED - OS_BASE_CACHED)))
#endif

#ifndef OS_SYS_CALL_HANDLER
#define OS_SYS_CALL_HANDLER  ((void*)0x80000C00)
#endif

#ifndef OS_HANDLER_SLOT_SIZE
#define OS_HANDLER_SLOT_SIZE 0x100
#endif

#ifndef OS_CONSOLE_RETAIL
#define OS_CONSOLE_RETAIL       0x00000000
#define OS_CONSOLE_RETAIL1      0x00000001
#define OS_CONSOLE_RETAIL2      0x00000002
#define OS_CONSOLE_RETAIL3      0x00000003
#define OS_CONSOLE_RETAIL4      0x00000004
#define OS_CONSOLE_EMULATOR     0x10000000
#define OS_CONSOLE_PC_EMULATOR  0x10000001
#define OS_CONSOLE_ARTHUR       0x10000002
#define OS_CONSOLE_MINNOW       0x10000003
#define OS_CONSOLE_DEVHW1       0x10000004
#define OS_CONSOLE_DEVHW2       0x10000005
#define OS_CONSOLE_DEVHW3       0x10000006
#define OS_CONSOLE_DEVHW4       0x10000007
#endif

#ifndef OS_EURGB60_OFF
#define OS_EURGB60_OFF 0u
#define OS_EURGB60_ON  1u
#endif

#ifndef RTC_CMD_READ
#define RTC_CMD_READ   0x20000000
#define RTC_CMD_WRITE  0xA0000000
#define RTC_SRAM_ADDR  0x00000100
#define RTC_SRAM_SIZE  64
#define RTC_CHAN       0
#define RTC_DEV        1
#define RTC_FREQ       3
#endif

#ifdef __cplusplus
}
#endif

#endif /* DOLPHIN_OS_UTIL_H */

