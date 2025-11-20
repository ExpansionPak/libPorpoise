#include "GXStubUtils.h"

#include <dolphin/gx/GXFifo.h>

static GXFifoObj* s_cpuFifo = NULL;
static GXFifoObj* s_gpFifo = NULL;

void GXInitFifoBase(GXFifoObj* fifo, void* base, u32 size) {
    GX_UNUSED(fifo);
    GX_UNUSED(base);
    GX_UNUSED(size);
    GX_STUB_NOTICE("GXInitFifoBase");
}

void GXInitFifoPtrs(GXFifoObj* fifo, void* readPtr, void* writePtr) {
    GX_UNUSED(fifo);
    GX_UNUSED(readPtr);
    GX_UNUSED(writePtr);
    GX_STUB_NOTICE("GXInitFifoPtrs");
}

void GXGetFifoPtrs(GXFifoObj* fifo, void** readPtr, void** writePtr) {
    GX_UNUSED(fifo);
    if (readPtr) *readPtr = NULL;
    if (writePtr) *writePtr = NULL;
    GX_STUB_NOTICE("GXGetFifoPtrs");
}

GXFifoObj* GXGetCPUFifo(void) {
    return s_cpuFifo;
}

GXFifoObj* GXGetGPFifo(void) {
    return s_gpFifo;
}

void GXSetCPUFifo(GXFifoObj* fifo) {
    s_cpuFifo = fifo;
    GX_STUB_NOTICE("GXSetCPUFifo");
}

void GXSetGPFifo(GXFifoObj* fifo) {
    s_gpFifo = fifo;
    GX_STUB_NOTICE("GXSetGPFifo");
}

void GXSaveCPUFifo(GXFifoObj* fifo) {
    GX_UNUSED(fifo);
    GX_STUB_NOTICE("GXSaveCPUFifo");
}

void GXGetFifoStatus(GXFifoObj* fifo, GXBool* overhi, GXBool* underlow, u32* fifoCount, GXBool* cpu_write,
                     GXBool* gp_read, GXBool* fifowrap) {
    GX_UNUSED(fifo);
    if (overhi) *overhi = GX_FALSE;
    if (underlow) *underlow = GX_FALSE;
    if (fifoCount) *fifoCount = 0;
    if (cpu_write) *cpu_write = GX_FALSE;
    if (gp_read) *gp_read = GX_FALSE;
    if (fifowrap) *fifowrap = GX_FALSE;
    GX_STUB_NOTICE("GXGetFifoStatus");
}

void GXGetGPStatus(GXBool* overhi, GXBool* underlow, GXBool* readIdle, GXBool* cmdIdle, GXBool* brkpt) {
    if (overhi) *overhi = GX_FALSE;
    if (underlow) *underlow = GX_FALSE;
    if (readIdle) *readIdle = GX_TRUE;
    if (cmdIdle) *cmdIdle = GX_TRUE;
    if (brkpt) *brkpt = GX_FALSE;
    GX_STUB_NOTICE("GXGetGPStatus");
}

void GXInitFifoLimits(GXFifoObj* fifo, u32 hiWaterMark, u32 loWaterMark) {
    GX_UNUSED(fifo);
    GX_UNUSED(hiWaterMark);
    GX_UNUSED(loWaterMark);
    GX_STUB_NOTICE("GXInitFifoLimits");
}


