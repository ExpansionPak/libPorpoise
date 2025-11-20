/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-matrix-create.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-matrix-create.c $
    
    1     9/19/01 5:49p Carl
    Source files for GD matrix demo.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <dolphin/gd.h>

#ifdef WIN32
#include <assert.h>
#include <stdlib.h>
#else
#include <dolphin/os.h>
#endif

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

#ifdef WIN32
#define ASSERT           assert
#define OSRoundUp32B(x)  (((u32)(x) + 31) & ~31)
#define OSAlloc(x)       ((void*)OSRoundUp32B(malloc((x)+31)))
#define OSFree(x)        free(x)
#endif

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void CreateDLs ( void );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Display lists *************************************************************

// This set of display lists will each load an indexed position matrix
// and an indexed normal matrix, then draw one face of the cube.

extern GDLObj DrawDLOs[6];
#define ALT_DRAW_SIZE 320  // maximum (not actual) size

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Name:           CreateDLs
    
    Description:    Creates the display lists used by the program.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void CreateDLs ( void )
{
    u8 *DrawList;

    //
    // Create the cube-face-drawing draw DL's
    //

    // This set of display lists will each load an indexed position matrix
    // and an indexed normal matrix, then draw one face of the cube.

    // Note that each DL draws a unique face.  Yes, we could have just
    // used one face and then altered the matrices appropriately, but
    // that would have been more work.

    // face 1

    DrawList = OSAlloc( ALT_DRAW_SIZE );
    ASSERT(DrawList);

    GDInitGDLObj( &DrawDLOs[0], DrawList, ALT_DRAW_SIZE );
    GDSetCurrent( &DrawDLOs[0] );

    GDLoadPosMtxIndx(0, GX_PNMTX0);     // Load position mtx for this face
    GDLoadNrmMtxIndx3x3(0, GX_PNMTX0);  // Load normal mtx for this face

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 ); // Draw face
    GDPosition1x8( 0 ); GDNormal1x8( 0 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 1 ); GDNormal1x8( 1 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 2 ); GDNormal1x8( 2 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 3 ); GDNormal1x8( 3 ); GDTexCoord1x8( 3 );
    GDEnd();

    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem();

    // face 2

    DrawList = OSAlloc( ALT_DRAW_SIZE );
    ASSERT(DrawList);

    GDInitGDLObj( &DrawDLOs[1], DrawList, ALT_DRAW_SIZE );
    GDSetCurrent( &DrawDLOs[1] );

    GDLoadPosMtxIndx(1, GX_PNMTX0);
    GDLoadNrmMtxIndx3x3(1, GX_PNMTX0);

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 4 ); GDNormal1x8( 4 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 5 ); GDNormal1x8( 5 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 6 ); GDNormal1x8( 6 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 7 ); GDNormal1x8( 7 ); GDTexCoord1x8( 3 );
    GDEnd();

    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem();

    // face 3

    DrawList = OSAlloc( ALT_DRAW_SIZE );
    ASSERT(DrawList);

    GDInitGDLObj( &DrawDLOs[2], DrawList, ALT_DRAW_SIZE );
    GDSetCurrent( &DrawDLOs[2] );

    GDLoadPosMtxIndx(2, GX_PNMTX0);
    GDLoadNrmMtxIndx3x3(2, GX_PNMTX0);

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 2 ); GDNormal1x8( 2 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 6 ); GDNormal1x8( 6 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 5 ); GDNormal1x8( 5 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 3 ); GDNormal1x8( 3 ); GDTexCoord1x8( 3 );
    GDEnd();

    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem();

    // face 4

    DrawList = OSAlloc( ALT_DRAW_SIZE );
    ASSERT(DrawList);

    GDInitGDLObj( &DrawDLOs[3], DrawList, ALT_DRAW_SIZE );
    GDSetCurrent( &DrawDLOs[3] );

    GDLoadPosMtxIndx(3, GX_PNMTX0);
    GDLoadNrmMtxIndx3x3(3, GX_PNMTX0);

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 1 ); GDNormal1x8( 1 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 0 ); GDNormal1x8( 0 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 4 ); GDNormal1x8( 4 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 7 ); GDNormal1x8( 7 ); GDTexCoord1x8( 3 );
    GDEnd();

    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem();

    // face 5

    DrawList = OSAlloc( ALT_DRAW_SIZE );
    ASSERT(DrawList);

    GDInitGDLObj( &DrawDLOs[4], DrawList, ALT_DRAW_SIZE );
    GDSetCurrent( &DrawDLOs[4] );

    GDLoadPosMtxIndx(4, GX_PNMTX0);
    GDLoadNrmMtxIndx3x3(4, GX_PNMTX0);

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 5 ); GDNormal1x8( 5 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 4 ); GDNormal1x8( 4 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 0 ); GDNormal1x8( 0 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 3 ); GDNormal1x8( 3 ); GDTexCoord1x8( 3 );
    GDEnd();

    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem();

    // face 6

    DrawList = OSAlloc( ALT_DRAW_SIZE );
    ASSERT(DrawList);

    GDInitGDLObj( &DrawDLOs[5], DrawList, ALT_DRAW_SIZE );
    GDSetCurrent( &DrawDLOs[5] );

    GDLoadPosMtxIndx(5, GX_PNMTX0);
    GDLoadNrmMtxIndx3x3(5, GX_PNMTX0);

    GDBegin( GX_QUADS, GX_VTXFMT0, 4 );
    GDPosition1x8( 6 ); GDNormal1x8( 6 ); GDTexCoord1x8( 0 );
    GDPosition1x8( 2 ); GDNormal1x8( 2 ); GDTexCoord1x8( 1 );
    GDPosition1x8( 1 ); GDNormal1x8( 1 ); GDTexCoord1x8( 2 );
    GDPosition1x8( 7 ); GDNormal1x8( 7 ); GDTexCoord1x8( 3 );
    GDEnd();

    // pad & flush
    GDPadCurr32();
    GDFlushCurrToMem();

    GDSetCurrent(NULL); // bug-prevention
}
