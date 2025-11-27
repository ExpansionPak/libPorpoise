#include <dolphin.h>
#include <win32/win32.h>

u32			ScreenWidth  = 640;
u32			ScreenHeight = 480;

void VIInit(void) {
}

void VIWaitForRetrace   ( void ) {
}

void VIConfigureTVScreen(u16 xOrg, u16 yOrg, u16 xSize, VITVMode mode) {
}

void VIConfigureXFrameBuffer(u16 xSize, u16 ySize, VIXFBMode mode) {
}

void VIConfigurePan(u16 xOrg, u16 yOrg, u16 width, u16 height) {
}

void VISetNextFrameBuffer (void *fb) {
}

void VISetFrameBuffer (void *fb) {
}

void VISetBlack(BOOL black) {
}

u32 VIGetRetraceCount(void) {
	return retraceCount;
}

u32 VIGetNextField(void) {
	return 0;
}

void VIFlush(void) {
}

void VIConfigure(GXRenderModeObj* rm) {
}

u32 VIGetTvFormat(void) {
	return VI_NTSC;;
}

VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback cb)
{
    return cb;
}

VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback cb)
{
    return cb;
}