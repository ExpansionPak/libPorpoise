/*---------------------------------------------------------------------------*
  Project:  libPorpoise Demo Library
  File:     DEMOPuts.c
  
  Text rendering - stub implementation for compilation.
  
  Based on Nintendo's Revolution SDK demo library.
 *---------------------------------------------------------------------------*/

#include <dolphin/demo/demo.h>
#include <dolphin/os.h>
#include <dolphin/gx.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Font bitmap (stub) */
u32 DEMOFontBitmap[1] = {0};

void DEMOSetFontType(s32 type) {
    (void)type;
    // Stub
}

void DEMOSetupScrnSpc(s32 x, s32 y, f32 z) {
    (void)x; (void)y; (void)z;
    // Stub
}

void DEMOInitCaption(s32 x, s32 y, s32 z) {
    (void)x; (void)y; (void)z;
    // Stub
}

void DEMOLoadFont(GXTexMapID id, GXTexMtx mtx, DMTexFlt flt) {
    (void)id; (void)mtx; (void)flt;
    // Stub
}

void DEMOPuts(s16 x, s16 y, s16 z, char* str) {
    (void)x; (void)y; (void)z; (void)str;
    // Stub - forward to OSReport for now
    if (str) {
        OSReport("%s\n", str);
    }
}

void DEMOPrintf(s16 x, s16 y, s16 z, char* fmt, ...) {
    (void)x; (void)y; (void)z;
    va_list args;
    va_start(args, fmt);
    // Stub - forward to OSReport for now
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    OSReport("%s\n", buffer);
    va_end(args);
}

OSFontHeader* DEMOInitROMFont(void) {
    // Stub
    return NULL;
}

void DEMOSetROMFontSize(s16 size, s16 space) {
    (void)size; (void)space;
    // Stub
}

void DEMOGetROMFontSize(s16* size, s16* space) {
    if (size) *size = 0;
    if (space) *space = 0;
    // Stub
}

int DEMOGetRFTextWidth(char* string) {
    (void)string;
    return 0; // Stub
}

int DEMOGetRFTextHeight(char* string) {
    (void)string;
    return 0; // Stub
}

int DEMORFPuts(s16 x, s16 y, s16 z, char* string) {
    (void)x; (void)y; (void)z;
    if (string) {
        OSReport("%s\n", string);
    }
    return 0; // Stub
}

int DEMORFPutsEx(s16 x, s16 y, s16 z, char* string, s16 maxWidth, int length) {
    (void)x; (void)y; (void)z; (void)maxWidth; (void)length;
    if (string) {
        OSReport("%s\n", string);
    }
    return 0; // Stub
}

int DEMORFPrintf(s16 x, s16 y, s16 z, char* fmt, ...) {
    (void)x; (void)y; (void)z;
    va_list args;
    va_start(args, fmt);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    OSReport("%s\n", buffer);
    va_end(args);
    return 0; // Stub
}

char* DEMODumpROMFont(char* string) {
    (void)string;
    return NULL; // Stub
}

