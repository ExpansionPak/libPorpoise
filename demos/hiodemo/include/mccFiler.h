/*---------------------------------------------------------------------*

Project:  mcc bitmap viewer - filer module
File:     mccFiler.h

(C) 2000 Nintendo

-----------------------------------------------------------------------*/

#ifndef	__MCC_BITMAP_VIEWER__FILER__
#define	__MCC_BITMAP_VIEWER__FILER__

// TYPE DEFINES
// ==============================
typedef u8 FILER_DEVICE;
#define FILER_DEVICE_DVD (u32)(0x00000001)
#define FILER_DEVICE_FIO (u32)(0x00000002)

// FUNCTION PROTOTYPES
// ==============================
void filerInitialize();
void filerSetDevice( FILER_DEVICE filerDevice );
void filerChangeDir(char *path);
void filerLoadFile(char *fileName);

#endif