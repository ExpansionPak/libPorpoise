#include "GXStubUtils.h"

#include <dolphin/gx/GXLighting.h>
#include "../gfx/gx_state.h"
#include "../gfx/gx_update_state.h"  // For update_gx_state

void GXInitLightAttn(GXLightObj* light_, float a0, float a1, float a2, float k0, float k1, float k2) {
    GX_UNUSED(light_);
    GX_UNUSED(a0);
    GX_UNUSED(a1);
    GX_UNUSED(a2);
    GX_UNUSED(k0);
    GX_UNUSED(k1);
    GX_UNUSED(k2);
    GX_STUB_NOTICE("GXInitLightAttn");
}

void GXInitLightAttnA(GXLightObj* light_, float a0, float a1, float a2) {
    GX_UNUSED(light_);
    GX_UNUSED(a0);
    GX_UNUSED(a1);
    GX_UNUSED(a2);
    GX_STUB_NOTICE("GXInitLightAttnA");
}

void GXInitLightAttnK(GXLightObj* light_, float k0, float k1, float k2) {
    GX_UNUSED(light_);
    GX_UNUSED(k0);
    GX_UNUSED(k1);
    GX_UNUSED(k2);
    GX_STUB_NOTICE("GXInitLightAttnK");
}

void GXInitLightSpot(GXLightObj* light_, float cutoff, GXSpotFn spotFn) {
    GX_UNUSED(light_);
    GX_UNUSED(cutoff);
    GX_UNUSED(spotFn);
    GX_STUB_NOTICE("GXInitLightSpot");
}

void GXInitLightDistAttn(GXLightObj* light_, float refDistance, float refBrightness, GXDistAttnFn distFunc) {
    GX_UNUSED(light_);
    GX_UNUSED(refDistance);
    GX_UNUSED(refBrightness);
    GX_UNUSED(distFunc);
    GX_STUB_NOTICE("GXInitLightDistAttn");
}

void GXInitLightPos(GXLightObj* light_, float x, float y, float z) {
    GX_UNUSED(light_);
    GX_UNUSED(x);
    GX_UNUSED(y);
    GX_UNUSED(z);
    GX_STUB_NOTICE("GXInitLightPos");
}

void GXInitLightDir(GXLightObj* light_, float nx, float ny, float nz) {
    GX_UNUSED(light_);
    GX_UNUSED(nx);
    GX_UNUSED(ny);
    GX_UNUSED(nz);
    GX_STUB_NOTICE("GXInitLightDir");
}

void GXInitSpecularDir(GXLightObj* light_, float nx, float ny, float nz) {
    GX_UNUSED(light_);
    GX_UNUSED(nx);
    GX_UNUSED(ny);
    GX_UNUSED(nz);
    GX_STUB_NOTICE("GXInitSpecularDir");
}

void GXInitSpecularDirHA(GXLightObj* light_, float nx, float ny, float nz, float hx, float hy, float hz) {
    GX_UNUSED(light_);
    GX_UNUSED(nx);
    GX_UNUSED(ny);
    GX_UNUSED(nz);
    GX_UNUSED(hx);
    GX_UNUSED(hy);
    GX_UNUSED(hz);
    GX_STUB_NOTICE("GXInitSpecularDirHA");
}

void GXInitLightColor(GXLightObj* light_, GXColor color) {
    GX_UNUSED(light_);
    GX_UNUSED(color);
    GX_STUB_NOTICE("GXInitLightColor");
}

void GXLoadLightObjImm(GXLightObj* light_, GXLightID id) {
    GX_UNUSED(light_);
    GX_UNUSED(id);
    GX_STUB_NOTICE("GXLoadLightObjImm");
}

void GXSetChanAmbColor(GXChannelID id, GXColor color) {
    GX_UNUSED(id);
    GX_UNUSED(color);
    GX_STUB_NOTICE("GXSetChanAmbColor");
}

void GXSetChanMatColor(GXChannelID id, GXColor color) {
    GX_UNUSED(id);
    GX_UNUSED(color);
    GX_STUB_NOTICE("GXSetChanMatColor");
}

void GXSetNumChans(u8 num) {
    /* Copy from Aurora exactly: GXSetNumChans uses update_gx_state */
    /* Aurora's code: update_gx_state(g_gxState.numChans, num); */
    update_gx_state(g_gxState->numChans, num);
}

void GXSetChanCtrl(GXChannelID id, GXBool lightingEnabled, GXColorSrc ambSrc, GXColorSrc matSrc, u32 lightMask,
                   GXDiffuseFn diffFn, GXAttnFn attnFn) {
    GX_UNUSED(id);
    GX_UNUSED(lightingEnabled);
    GX_UNUSED(ambSrc);
    GX_UNUSED(matSrc);
    GX_UNUSED(lightMask);
    GX_UNUSED(diffFn);
    GX_UNUSED(attnFn);
    GX_STUB_NOTICE("GXSetChanCtrl");
}


