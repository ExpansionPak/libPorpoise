/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     G2D-testEditor.c (Test of 2D API by Paul Donnelly, Nov. 1999)

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/G2D/G2D-testEditor.c $
    
    3     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    2     3/23/00 2:10a Hirose
    updated to use DEMOPad library / deleted all tab codes
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     1/18/00 8:04p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls to work with
    new GXInit() defaults
    
    3     99/12/15 2:14p Paul
    Added a switch to turn off map-editor mode
    
    2     99/12/14 8:34a Paul
    
    1     99/12/12 10:07p Paul
    

 *---------------------------------------------------------------------------*/

#include "G2D-test.h"

#ifdef _EDITOR
    #include <demo.h>
    #include <math.h>
    #include <string.h>

    /*---------------------------------------------------------------------------*
       Defines
     *---------------------------------------------------------------------------*/


    /*---------------------------------------------------------------------------*
       Global Variables
     *---------------------------------------------------------------------------*/

    s32 nMapX, nMapY;
    s32 nStampLeft, nStampWidth = 1;
    s32 nStampTop, nStampHeight = 1;
    s32 nStampStartX, nStampStartY;
    s32 nStampOffsetX, nStampOffsetY;
    u8 aEditStamp[8192];
    u8 aEditBack[8192];

    /*---------------------------------------------------------------------------*
       Local Function Declarations
     *---------------------------------------------------------------------------*/

    static void DrawCursor          ( void );

#endif // ifdef _EDITOR

/*---------------------------------------------------------------------------*
    Name:           MapEditor

    Description:    Logic for map editor mode
                    (uses ship position variables for cursor position)

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
void MapEditor( G2DLayer *layer )
{
#ifdef _EDITOR

    s8 nDX, nDY;
    static s8 nOldDX = 0;
    static s8 nOldDY = 0;
    s32 nMapIdx, nIdx;
    u8 *buf = aEditStamp;
    s32 nX, nY, nZ;
    s32 nOldMapX, nOldMapY;

    nOldMapX= nMapX;
    nOldMapY= nMapY;
    nDX = 0;
    if (DEMOPadGetDirsNew(0) & DEMO_STICK_RIGHT)
    {
        nDX = 1;
    }
    else if (DEMOPadGetDirsNew(0) & DEMO_STICK_LEFT)
    {
        nDX = -1;
    }

    nDY = 0;
    if (DEMOPadGetDirsNew(0) & DEMO_STICK_DOWN)
    {
        nDY = 1;
    }
    else if (DEMOPadGetDirsNew(0) & DEMO_STICK_UP)
    {
        nDY = -1;
    }

    if (nButtons & PAD_BUTTON_B)
    {
        poShip.rPosX += (f32)(nDX * layer->nTileWidth);
        poShip.rPosY += (f32)(nDY * layer->nTileHeight);
    }
    else
    {
        if ((nDX) && (nOldDX == 0))
        {
            poShip.rPosX += (f32)(nDX * layer->nTileWidth);
        }

        if ((nDY) && (nOldDY == 0))
        {
            poShip.rPosY += (f32)(nDY * layer->nTileHeight);
        }
    }

    nOldDX = nDX;
    nOldDY = nDY;

    if (poShip.rPosX >= rWorldWidth)  { poShip.rPosX -= rWorldWidth; }
    else if (poShip.rPosX < 0)        { poShip.rPosX += rWorldWidth; }
    if (poShip.rPosY >= rWorldHeight) { poShip.rPosY -= rWorldHeight; }
    else if (poShip.rPosY < 0)        { poShip.rPosY += rWorldHeight; }

    nMapX = (s32)(poShip.rPosX / layer->nTileWidth);
    nMapY = (s32)(poShip.rPosY / layer->nTileHeight);
    nMapIdx = (nMapY<<layer->nHS) + nMapX;

    if (nButtons & PAD_BUTTON_X)
    {
        if ((~nOldButtons) & PAD_BUTTON_X)
        {
            nStampStartX = nMapX;
            nStampStartY = nMapY;
            nStampOffsetX = 0;
            nStampOffsetY = 0;
        }
        else
        {
            /*  If there was a change in map coordinates, we should add
             *  that change to the stamp offset
             */
            if (nMapX != nOldMapX)
            {
                nStampOffsetX += nDX;
            }
            if (nMapY != nOldMapY)
            {
                nStampOffsetY += nDY;
            }
        }

        if (nStampOffsetX < 0)
        {
            nStampLeft = nStampStartX + nStampOffsetX;
            nStampWidth = 1 - nStampOffsetX;
        }
        else
        {
            nStampLeft = nStampStartX;
            nStampWidth = 1 + nStampOffsetX;
        }

        if (nStampOffsetY < 0)
        {
            nStampTop = nStampStartY + nStampOffsetY;
            nStampHeight = 1 - nStampOffsetY;
        }
        else
        {
            nStampTop = nStampStartY;
            nStampHeight = 1 + nStampOffsetY;
        }

        if (nStampWidth > nMapWidth)
        {
            nStampWidth = nMapWidth;
        }

        if (nStampHeight > nMapHeight)
        {
            nStampHeight = nMapHeight;
        }

        /* Copy block of tiles for use as a stamp */
        buf = aEditStamp;
        for(nY = 0; nY<nStampHeight; nY++)
        {
            nZ = ((nY + nStampTop) & (nMapHeight-1)) * nMapWidth;
            for(nX = 0; nX<nStampWidth; nX++)
            {
                *buf++ = map[nZ + ((nX + nStampLeft) & (nMapWidth-1))];
            }
        }
    }

    if ((nStampWidth == 1) && (nStampHeight == 1))
    {
        nIdx = (s32)(strchr(sUsedTiles, aEditStamp[0] + ' ') - sUsedTiles);

        if (((~nOldButtons & PAD_TRIGGER_L) | (nButtons & PAD_BUTTON_B)) &&
            (nButtons & PAD_TRIGGER_L))
        {
            if (nIdx == 0)
            {
                nIdx = USED_TILES;
            }
            nIdx--;

            aEditStamp[0] = (u8)(sUsedTiles[nIdx] - ' ');
        }

        if (((~nOldButtons & PAD_TRIGGER_R) | (nButtons & PAD_BUTTON_B)) &&
            (nButtons & PAD_TRIGGER_R))
        {
            nIdx++;
            if (nIdx == USED_TILES)
            {
                nIdx = 0;
            }

            aEditStamp[0] = (u8)(sUsedTiles[nIdx] - ' ');
        }
    }

    if (nButtons & PAD_BUTTON_A)
    {
        s32 nStaticX = nStampLeft + nMapX - nStampStartX - nStampOffsetX;
        s32 nStaticY = nStampTop + nMapY - nStampStartY - nStampOffsetY;

        buf = aEditStamp;
        
        /* Copy stamp into map */

        for(nY = 0; nY<nStampHeight; nY++)
        {
            nZ = ((nY + nStaticY) & (nMapHeight-1)) * nMapWidth;
            for(nX = 0; nX<nStampWidth; nX++)
            {
                map[nZ + ((nX + nStaticX) & (nMapWidth-1))] = *buf++;
            }
        }
    }

#else
    #pragma unused (layer)
#endif // ifdef _EDITOR
}


/*---------------------------------------------------------------------------*
    Name:           SaveMap

    Description:    Saves map data to a file

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
void SaveMap( G2DLayer *layer )
{
#ifdef _EDITOR

    FILE *fpMap = fopen("map.c", "wb");
    s32 nI, nJ;
    s32 nMapIdx;

    nMapIdx = (nMapY<<layer->nHS) + nMapX;
    map[nMapIdx] = (u8)('*'-' ');  // "*' indicates the start position of the ship

    for (nJ=0; nJ<nMapHeight; nJ++)
    {
        fprintf(fpMap, "\"");
        for (nI=0; nI<nMapWidth; nI++)
        {
            fprintf(fpMap, "%c", map[(nJ<<layer->nHS)+nI] + ' ');
        }
        fprintf(fpMap, "\"\n");
    }

    map[nMapIdx] = '.'-' ';

    fclose(fpMap);

#else
    #pragma unused (layer)
#endif // ifdef _EDITOR
}


/*---------------------------------------------------------------------------*
    Name:           DrawCursor

    Description:    Draw the map-editor cursor

    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCursor( void )
{
#ifdef _EDITOR

    s32 nX, nY, nW, nH;

    nX = lyrBack.nTileWidth *  (nStampLeft + nMapX - nStampStartX - nStampOffsetX);
    nY = lyrBack.nTileHeight * (nStampTop + nMapY - nStampStartY - nStampOffsetY);
    nW = lyrBack.nTileWidth * nStampWidth;
    nH = lyrBack.nTileHeight * nStampHeight;

    /* Draw a white box around the position of the cursor */

    // Set Position Params
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0); //PositionShift);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    // Set Color for Vertex Format 0
    GXSetNumTexGens(0);
    GXSetNumChans(1);
    GXSetVtxDesc(GX_VA_TEX0, GX_NONE);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);


    GXBegin(GX_LINESTRIP, GX_VTXFMT0, 3);
        GXPosition2f32( (f32)(nX + 4), (f32)nY );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)nX, (f32)nY );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)nX, (f32)(nY + 4) );
        GXColor3u8( 255, 255, 255 );
    GXEnd();

    GXBegin(GX_LINESTRIP, GX_VTXFMT0, 3);
        GXPosition2f32( (f32)(nX + nW - 4), (f32)nY );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)(nX + nW), (f32)nY );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)(nX + nW), (f32)(nY + 4) );
        GXColor3u8( 255, 255, 255 );
    GXEnd();

    GXBegin(GX_LINESTRIP, GX_VTXFMT0, 3);
        GXPosition2f32( (f32)(nX + 4), (f32)(nY + nH) );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)nX, (f32)(nY + nH) );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)nX, (f32)(nY + nH - 4) );
        GXColor3u8( 255, 255, 255 );
    GXEnd();

    GXBegin(GX_LINESTRIP, GX_VTXFMT0, 3);
        GXPosition2f32( (f32)(nX + nW - 4), (f32)(nY + nH) );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)(nX + nW), (f32)(nY + nH) );
        GXColor3u8( 255, 255, 255 );
        GXPosition2f32( (f32)(nX + nW), (f32)(nY + nH - 4) );
        GXColor3u8( 255, 255, 255 );
    GXEnd();
#endif // ifdef _EDITOR
}


/*---------------------------------------------------------------------------*
    Name:           RenderEditorMode

    Description:    Renders the 

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
void RenderEditorMode( s8 *aSortBuffer )
{
#ifdef _EDITOR

    s32 nStaticX = nStampLeft + nMapX - nStampStartX - nStampOffsetX;
    s32 nStaticY = nStampTop + nMapY - nStampStartY - nStampOffsetY;
    s32 nMapWidth = 1<<lyrBack.nHS;
    s32 nMapHeight = 1<<lyrBack.nVS;
    s32 nX, nY, nZ;
    u8 *buf;

    /* Copy from map to back buffer */
    buf = aEditBack;
    for(nY = 0; nY<nStampHeight; nY++)
    {
        nZ = ((nY + nStaticY) & (nMapHeight-1)) * nMapWidth;
        for(nX = 0; nX<nStampWidth; nX++)
        {
            *buf++ = map[nZ + ((nX + nStaticX) & (nMapWidth-1))];
        }
    }

    /* Copy stamp into map */
    buf = aEditStamp;
    for(nY = 0; nY<nStampHeight; nY++)
    {
        nZ = ((nY + nStaticY) & (nMapHeight-1)) * nMapWidth;
        for(nX = 0; nX<nStampWidth; nX++)
        {
            map[nZ + ((nX + nStaticX) & (nMapWidth-1))] = *buf++;
        }
    }

    G2DDrawLayer( &lyrBack, aSortBuffer );

    /* Copy from back buffer into map */
    buf = aEditBack;
    for(nY = 0; nY<nStampHeight; nY++)
    {
        nZ = ((nY + nStaticY) & (nMapHeight-1)) * nMapWidth;
        for(nX = 0; nX<nStampWidth; nX++)
        {
            map[nZ + ((nX + nStaticX) & (nMapWidth-1))] = *buf++;
        }
    }

    DrawCursor();

#else
    #pragma unused (aSortBuffer)
#endif // ifdef _EDITOR
}
