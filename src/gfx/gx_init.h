/*---------------------------------------------------------------------------*
  gx_init.h - GX Graphics System Initialization Header
 *---------------------------------------------------------------------------*/

#ifndef GX_INIT_H
#define GX_INIT_H

#include <dolphin/types.h>

/* Initialize the OpenGL graphics backend */
BOOL GXInitGraphics(void);

/* Shutdown the graphics backend */
void GXShutdownGraphics(void);

#endif

