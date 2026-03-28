/*---------------------------------------------------------------------------*
  PPCArch.c - PowerPC Architecture Functions for libPorpoise
  
  PC-compatible stubs for PowerPC architecture register access functions.
  On PC, these functions are stubs that return dummy values or are no-ops,
  providing API compatibility for games that use these functions.
 *---------------------------------------------------------------------------*/

#include <dolphin/types.h>
#include <dolphin/base/PPCArch.h>

#ifdef _WIN32
#include <windows.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#else
#include <unistd.h>
#endif

/* Static storage for emulated register values */
static u32 s_msr = MSR_EE | MSR_ME | MSR_IR | MSR_DR;
static u32 s_hid0 = HID0_ICE | HID0_DCE;
static u32 s_hid1 = 0;
static u32 s_hid2 = 0;
static u32 s_hid4 = 0;
static u32 s_l2cr = 0;
static u32 s_dec = 0;
static u32 s_mmcr0 = 0;
static u32 s_mmcr1 = 0;
static u32 s_pmc1 = 0;
static u32 s_pmc2 = 0;
static u32 s_pmc3 = 0;
static u32 s_pmc4 = 0;
static u32 s_sia = 0;
static u32 s_wpar = 0;
static u32 s_dmaU = 0;
static u32 s_dmaL = 0;
static u32 s_fpscr = 0;

/*---------------------------------------------------------------------------*
  MSR functions
 *---------------------------------------------------------------------------*/
u32 PPCMfmsr(void) {
    return s_msr;
}

void PPCMtmsr(u32 newMSR) {
    s_msr = newMSR;
}

u32 PPCOrMsr(u32 value) {
    s_msr |= value;
    return s_msr;
}

u32 PPCAndMsr(u32 value) {
    s_msr &= value;
    return s_msr;
}

u32 PPCAndCMsr(u32 value) {
    s_msr &= ~value;
    return s_msr;
}

/*---------------------------------------------------------------------------*
  HID0 functions
 *---------------------------------------------------------------------------*/
u32 PPCMfhid0(void) {
    return s_hid0;
}

void PPCMthid0(u32 newHID0) {
    s_hid0 = newHID0;
}

/*---------------------------------------------------------------------------*
  HID1 functions
 *---------------------------------------------------------------------------*/
u32 PPCMfhid1(void) {
    return s_hid1;
}

/*---------------------------------------------------------------------------*
  HID2 functions (GEKKO)
 *---------------------------------------------------------------------------*/
u32 PPCMfhid2(void) {
    return s_hid2;
}

void PPCMthid2(u32 newhid2) {
    s_hid2 = newhid2;
}

/*---------------------------------------------------------------------------*
  WPAR functions (GEKKO)
 *---------------------------------------------------------------------------*/
u32 PPCMfwpar(void) {
    return s_wpar;
}

void PPCMtwpar(u32 newwpar) {
    s_wpar = newwpar;
}

/*---------------------------------------------------------------------------*
  DMA functions (GEKKO)
 *---------------------------------------------------------------------------*/
u32 PPCMfdmaU(void) {
    return s_dmaU;
}

u32 PPCMfdmaL(void) {
    return s_dmaL;
}

void PPCMtdmaU(u32 newdmau) {
    s_dmaU = newdmau;
}

void PPCMtdmaL(u32 newdmal) {
    s_dmaL = newdmal;
}

/*---------------------------------------------------------------------------*
  L2CR functions
 *---------------------------------------------------------------------------*/
u32 PPCMfl2cr(void) {
    return s_l2cr;
}

void PPCMtl2cr(u32 newL2cr) {
    s_l2cr = newL2cr;
}

/*---------------------------------------------------------------------------*
  DEC functions
 *---------------------------------------------------------------------------*/
void PPCMtdec(u32 newDec) {
    s_dec = newDec;
}

u32 PPCMfdec(void) {
    return s_dec;
}

/*---------------------------------------------------------------------------*
  Sync functions
 *---------------------------------------------------------------------------*/
void PPCSync(void) {
    /* Approximate PPC "sync" with a full host memory fence. */
    #if defined(_MSC_VER)
    _ReadWriteBarrier();
    MemoryBarrier();
    #elif defined(__GNUC__) || defined(__clang__)
    __sync_synchronize();
    #else
    /* Fallback: keep API behavior if no barrier primitive is available. */
    #endif
}

void PPCEieio(void) {
    /* On PC, I/O ordering is handled by the OS/CPU automatically */
    /* This is a no-op for compatibility */
}

/*---------------------------------------------------------------------------*
  Halt function
 *---------------------------------------------------------------------------*/
void PPCHalt(void) {
    /* On PC, halt the process */
    #ifdef _WIN32
    ExitProcess(0);
    #else
    _exit(0);
    #endif
}

/*---------------------------------------------------------------------------*
  Performance Monitor functions
 *---------------------------------------------------------------------------*/
u32 PPCMfmmcr0(void) {
    return s_mmcr0;
}

void PPCMtmmcr0(u32 newMmcr0) {
    s_mmcr0 = newMmcr0;
}

u32 PPCMfmmcr1(void) {
    return s_mmcr1;
}

void PPCMtmmcr1(u32 newMmcr1) {
    s_mmcr1 = newMmcr1;
}

u32 PPCMfpmc1(void) {
    return s_pmc1;
}

void PPCMtpmc1(u32 newPmc1) {
    s_pmc1 = newPmc1;
}

u32 PPCMfpmc2(void) {
    return s_pmc2;
}

void PPCMtpmc2(u32 newPmc2) {
    s_pmc2 = newPmc2;
}

u32 PPCMfpmc3(void) {
    return s_pmc3;
}

void PPCMtpmc3(u32 newPmc3) {
    s_pmc3 = newPmc3;
}

u32 PPCMfpmc4(void) {
    return s_pmc4;
}

void PPCMtpmc4(u32 newPmc4) {
    s_pmc4 = newPmc4;
}

u32 PPCMfsia(void) {
    return s_sia;
}

void PPCMtsia(u32 newSia) {
    s_sia = newSia;
}

/*---------------------------------------------------------------------------*
  PVR function
 *---------------------------------------------------------------------------*/
u32 PPCMfpvr(void) {
    /* Return a fake PVR value indicating this is a PC port */
    /* Original GC: 0x00083214, Wii: 0x000C0200 */
    return 0x000C0200;  /* Return Wii PVR for compatibility */
}

/*---------------------------------------------------------------------------*
  FPSCR functions
 *---------------------------------------------------------------------------*/
u32 PPCMffpscr(void) {
    return s_fpscr;
}

void PPCMtfpscr(u32 newFPSCR) {
    s_fpscr = newFPSCR;
}

/*---------------------------------------------------------------------------*
  Mode functions
 *---------------------------------------------------------------------------*/
void PPCEnableSpeculation(void) {
    s_hid0 &= ~HID0_SPD;
}

void PPCDisableSpeculation(void) {
    s_hid0 |= HID0_SPD;
}

void PPCSetFpIEEEMode(void) {
    s_fpscr &= ~FPSCR_NI;
}

void PPCSetFpNonIEEEMode(void) {
    s_fpscr |= FPSCR_NI;
}

/*---------------------------------------------------------------------------*
  Broadway HID4 functions
 *---------------------------------------------------------------------------*/
u32 PPCMfhid4(void) {
    return s_hid4;
}

void PPCMthid4(u32 newhid4) {
    s_hid4 = newhid4;
}

