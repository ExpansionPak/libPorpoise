# PPCArthur Patterns in libPorpoise

This document describes how libPorpoise incorporates patterns from Nintendo's PPCArthur development emulation system.

## What is PPCArthur?

PPCArthur was Nintendo's official development tool for running GameCube software on PowerMac development machines before deploying to the console. It used exception-based register trapping to emulate hardware registers in software.

## Key Patterns We've Adopted

### 1. OSAlarm-Based Interrupt Simulation

**PPCArthur Approach:**
- Uses `OSAlarm` periodic timers to simulate hardware interrupts (60Hz VBlank, SI transfers, etc.)
- More accurate than threads because alarms are scheduled by the OS timer system
- Matches original hardware behavior where interrupts fire at precise intervals

**libPorpoise Implementation:**
```c
// VI module - VBlank simulation (like PPCArthur RetraceEmulator)
static OSAlarm s_retraceAlarm;
static void RetraceEmulator(OSAlarm* alarm, OSContext* context) {
    // Toggle field for interlaced mode
    s_field ^= 1;
    
    // Set interrupt status
    s_piIntsr |= PI_INTSR_REG_VIINT_MASK;
    
    // Call callbacks
    if (s_preRetraceCallback) {
        s_preRetraceCallback(s_retraceCount);
    }
}

static void InitVIRetrace(void) {
    OSCreateAlarm(&s_retraceAlarm);
    OSTime frameTime = OSMicrosecondsToTicks(1000000 / 60);  // 60Hz
    OSSetPeriodicAlarm(&s_retraceAlarm, OSGetTime(), frameTime, RetraceEmulator);
}
```

**Benefits:**
- More accurate timing than thread-based sleep loops
- Uses same OS timer system as original hardware (OSAlarm)
- Easier to adjust timing based on TV mode (50Hz/60Hz)

### 2. Register State Tracking

**PPCArthur Approach:**
- Maintains emulated register arrays (e.g., `__VIEmuRegs`, `__SIEmuRegs`, `__EXIEmuRegs`)
- Tracks hardware register state for compatibility
- Games can read/write registers and get expected behavior

**libPorpoise Implementation:**
```c
// VI module - Register state tracking
typedef struct {
    u16 dsp_cfg;        // VI_DSP_CFG - Display configuration
    u16 dsp_pos_u;      // VI_DSP_POS_U - Display position upper
    u16 dsp_pos_l;      // VI_DSP_POS_L - Display position lower
    u16 dsp_int0_u;     // VI_DSP_INT0_U - Interrupt 0 upper
    u16 dsp_int1_u;     // VI_DSP_INT1_U - Interrupt 1 upper
    // ... etc
} VIRegisterState;

static VIRegisterState s_viRegs = {0};
static u32 s_piIntsr = 0;  // PI interrupt status register
```

**Benefits:**
- Games that read VI registers get valid values
- Better compatibility with low-level hardware access
- Can track interrupt status and register changes

### 3. Field-Based Interlaced Rendering

**PPCArthur Approach:**
- Tracks current field (above/below) for interlaced modes
- Alternates fields each retrace (60Hz = 30 fields/sec per eye)
- Calculates display position based on field and time

**libPorpoise Implementation:**
```c
static u32 s_field = 0;  // Current field (0=below, 1=above)

static void RetraceEmulator(OSAlarm* alarm, OSContext* context) {
    // Toggle field for interlaced mode (like PPCArthur)
    s_field ^= 1;
    
    if (s_field == 0) {
        // Field below - set INT0
        s_viRegs.dsp_int0_u |= 0x0001;
    } else {
        // Field above - set INT1
        s_viRegs.dsp_int1_u |= 0x0001;
        s_frameStart = OSGetTime();
    }
}

u32 VIGetNextField(void) {
    return s_field ? VI_FIELD_ABOVE : VI_FIELD_BELOW;
}
```

**Benefits:**
- Accurate field tracking for interlaced modes
- Games that check field get correct values
- Better simulation of original hardware behavior

### 4. Position Calculation

**PPCArthur Approach:**
- Calculates current scan line based on time since frame start
- Uses hardware timing (74ns per dot, 429 dots per half-line)
- Provides accurate VI_DSP_POS register values

**libPorpoise Implementation:**
```c
u32 VIGetCurrentLine(void) {
    if (s_frameStart == 0) {
        return 0;
    }
    
    OSTime elapsed = OSGetTime() - s_frameStart;
    u32 nanoseconds = (u32)OSTicksToNanoseconds(elapsed);
    
    // Calculate line based on TV mode
    // NTSC: 262.5 lines per field, ~74ns per dot, ~429 dots per half-line
    u32 dotsPerHalfLine = 429;
    u32 dotsPerField = 262 * 2 * dotsPerHalfLine;
    u32 dot = (nanoseconds / 74) % dotsPerField;
    u32 line = (dot / (2 * dotsPerHalfLine)) + 1;
    
    return (line > 262) ? 262 : line;
}
```

**Benefits:**
- Games that read current line get accurate values
- Better simulation of hardware scan-out behavior
- Can be used for timing-sensitive operations

## Completed Improvements

### 1. SI Module Register Tracking ✅

**PPCArthur Pattern:**
- Tracks SI register state (`__SIEmuRegs`)
- Simulates controller polling with OSAlarm timers
- Handles command/response timing

**libPorpoise Implementation:**
- ✅ Added SI register state tracking (`s_siRegs[]`)
- ✅ OSAlarm-based transfer timing simulation
- ✅ Tracks COMCSR, SR, OUTBUF, INBUF registers
- ✅ State transition detection (when TSTART is set)
- ✅ Interrupt status tracking (PI_INTSR)
- ✅ Transfer timing calculation (7600ns + 1050ns + 2000ns + 1050ns + 4200ns * 8 * bytes)

**Code Example:**
```c
// SI register state (like PPCArthur __SIEmuRegs)
static u32 s_siRegs[SI_REG_MAX] = {0};

// Transfer timing simulation
static void TransferAlarmHandler(OSAlarm* alarm, OSContext* context) {
    // Clear TSTART bit (transfer complete)
    s_siRegs[SI_REG_COMCSR] &= ~SI_COMCSR_TSTART;
    
    // Set TCINT bit (transfer complete interrupt)
    s_siRegs[SI_REG_COMCSR] |= SI_COMCSR_TCINT;
    
    // Set interrupt status if enabled
    if ((s_siRegs[SI_REG_COMCSR] & SI_COMCSR_TCINT) &&
        (s_siRegs[SI_REG_COMCSR] & SI_COMCSR_TCINTMSK)) {
        s_piIntsr |= PI_INTSR_REG_SIINT_MASK;
    }
}

// Transfer timing calculation (like PPCArthur)
static OSTime CalculateTransferTime(u32 outBytes, u32 inBytes) {
    u64 baseTime = 11700;  // Base overhead (nanoseconds)
    u64 byteTime = 33600;  // Per byte (nanoseconds)
    u64 totalTime = baseTime + (byteTime * (outBytes + inBytes));
    return OSNanosecondsToTicks(totalTime);
}
```

### 2. EXI Module Register Tracking

**PPCArthur Pattern:**
- Tracks EXI register state (`__EXIEmuRegs`)
- Simulates memory card/network adapter timing
- Handles DMA transfers and interrupts

**libPorpoise TODO:**
- Add EXI register state tracking
- Simulate transfer timing with OSAlarm
- Track CPR, MAR, LENGTH, CR, DATA registers

### 3. DVD/DI Module Register Tracking

**PPCArthur Pattern:**
- Tracks DI register state (`__DIEmuRegs`)
- Simulates disc read timing
- Handles command completion interrupts

**libPorpoise TODO:**
- Add DI register state tracking
- Simulate read timing with OSAlarm
- Track SR, CVR, MAR, LENGTH, CR registers

### 4. PI Module Register Tracking

**PPCArthur Pattern:**
- Tracks PI register state (`__PIEmuRegs`)
- Manages interrupt status (INTSR, INTMSK)
- Handles CP (Command Processor) interrupts

**libPorpoise TODO:**
- Add PI register state tracking
- Track interrupt status across modules
- Simulate CP interrupt timing

## Architecture Differences

### PPCArthur (Exception-Based)
- **CPU:** PowerPC (same as GameCube)
- **Method:** Exception trapping (DSI handler catches register access)
- **Compatibility:** Can run same binary
- **Portability:** PowerMac only

### libPorpoise (API-Based)
- **CPU:** Any (x86, ARM, etc.)
- **Method:** API compatibility (function calls instead of registers)
- **Compatibility:** Requires recompilation
- **Portability:** Windows, Linux, macOS

## What We Can't Use

### Exception-Based Register Trapping
- Requires PowerPC CPU
- Can't trap register access on x86/ARM
- Not portable across platforms

### Hardware Register Access
- Games access registers directly (e.g., `*(u32*)PI_REGSP_VI`)
- On PC, these addresses don't exist
- Must use API functions instead

### Hardware Interrupt Handling
- Original uses hardware interrupts (VI, SI, DI, etc.)
- On PC, OS handles interrupts at kernel level
- Must simulate with timers/callbacks

## Summary

PPCArthur patterns provide valuable insights into:
1. **Timing simulation** - OSAlarm for accurate interrupt timing
2. **State tracking** - Register arrays for hardware state
3. **Field tracking** - Interlaced mode field management
4. **Position calculation** - Scan line timing based on hardware specs

libPorpoise adapts these patterns for cross-platform compatibility while maintaining the same programming model for games.

