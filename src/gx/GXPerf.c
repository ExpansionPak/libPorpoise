#include "GXStubUtils.h"

#include <dolphin/gx/GXPerf.h>

void GXReadXfRasMetric(u32* xf_wait_in, u32* xf_wait_out, u32* ras_busy, u32* clocks) {
    if (xf_wait_in) *xf_wait_in = 0;
    if (xf_wait_out) *xf_wait_out = 0;
    if (ras_busy) *ras_busy = 0;
    if (clocks) *clocks = 0;
    GX_STUB_NOTICE("GXReadXfRasMetric");
}


