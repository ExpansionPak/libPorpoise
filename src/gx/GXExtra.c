#include "GXStubUtils.h"

#include <dolphin/gx/GXExtra.h>

void GXDestroyTexObj(GXTexObj* obj) {
    GX_UNUSED(obj);
    GX_STUB_NOTICE("GXDestroyTexObj");
}

void GXDestroyTlutObj(GXTlutObj* obj) {
    GX_UNUSED(obj);
    GX_STUB_NOTICE("GXDestroyTlutObj");
}
