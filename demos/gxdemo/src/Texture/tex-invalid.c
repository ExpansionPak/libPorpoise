/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tex-invalid.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Texture/tex-invalid.c $
    
    13    5/11/01 1:00a Hirose
    removed obsolete flags. fixed some verify messages.
    
    12    7/07/00 5:57p Dante
    PC Compatibility
    
    11    6/15/00 4:04p Hirose
    fixed the synchronization bug
    
    10    6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    9     4/26/00 4:54p Carl
    CallBack -> Callback
    
    8     3/26/00 7:32p Hirose
    centered display object more considering H/V overscan again
    
    7     3/24/00 10:54p Hirose
    adjusted scene more considering heavy H/V overscan
    
    6     3/24/00 6:20p Carl
    Adjusted screen height for overscan.
    
    5     3/24/00 4:30p Hirose
    changed to use DEMOPad library
    
    4     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    3     3/22/00 3:31p Carl
    Adjusted for overscan
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:13p Alligator
    move from tests/gx and rename
    
    7     2/25/00 3:23p Hirose
    updated pad control functions to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     1/24/00 6:31p Hirose
    removed #ifdef EPPC stuff / changed to use 2D screen space
    
    4     1/21/00 6:23p Hirose
    added GXNumChans() call
    
    3     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     1/10/00 7:27p Hirose
    added single frame test
    
    1     1/10/00 6:30p Hirose
    initial version
  
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tex-invalid
        texture cache region invalidation test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <string.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define NUM_REGIONS     8
#define NUM_TEXOBJS     NUM_REGIONS
#define NUM_ROWS        5
#define NUM_TEXTURES    NUM_ROWS

// for display position
#define ALI_ROW(y)      (y * 80 + 56)
#define ALI_COL(x)      (x * 68 + 48) 

/*---------------------------------------------------------------------------*
   Structure definitions
 *---------------------------------------------------------------------------*/
// for entire scene control
typedef struct
{
    u32            curX;
    u32            curY;
    u8             invalidateSw[NUM_ROWS-2][NUM_REGIONS];
    GXTexObj       texture[NUM_TEXOBJS];
    u32            dataSize;
    u8*            dataBuffer[NUM_TEXOBJS];
    u8*            dataSource[NUM_TEXTURES];
    u16            screenWd;
    u16            screenHt;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main               ( void );
static void DrawInit           ( MySceneCtrlObj* sc );
static void DrawTick           ( MySceneCtrlObj* sc );
static void AnimTick           ( MySceneCtrlObj* sc );
static void DrawQuad           ( void );
static void DrawLines          ( void );
static void MyDrawCaption        ( MySceneCtrlObj* sc );
static void MyMemCopy          ( u8* src, u8* dst, u32 size );
static GXTexRegion* 
            MyTexRegionCallback( GXTexObj* texObj, GXTexMapID mapID );

static void PrintIntro         ( void );


/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
static s16 QuadVertices[3*8] ATTRIBUTE_ALIGN(32) = 
{
      0,   0,  0, // 0
     64,   0,  0, // 1
     64,  64,  0, // 2
      0,  64,  0  // 3
};

static s8 QuadTexCoords[2*4] ATTRIBUTE_ALIGN(32) =
{
      0,   0,
      1,   0,
      1,   1,
      0,   1
};

/*---------------------------------------------------------------------------*
   String data for messages
 *---------------------------------------------------------------------------*/
static char* InvSwStr[2] =
{
    "-------",
    "INVALID"
};

static char* InvTexAllStr = "******* GXInvalidateTexAll *******";

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;                 // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;           // texture palette
static GXTexRegion      MyTexRegions[NUM_REGIONS]; // cache regions

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats, array pointers
                          // and default scene settings.

    PrintIntro();    // Print demo directions

    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();           // Read controller
        AnimTick(&SceneCtrl);    // Do animation
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and sets up
                    the array pointer for the indexed data.
                    This function also initializes scene control parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{
    TEXDescriptorPtr  tdp;
    GXRenderModeObj*  rmode;
    u32               i, j;
    
    // Vertex Attribute (VTXFMT0 is used by DEMOPuts library)
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_S8, 0);

    // Array pointers
    GXSetArray(GX_VA_POS,  QuadVertices, 3 * sizeof(s16));
    GXSetArray(GX_VA_TEX0, QuadTexCoords, 2 * sizeof(s8));

    // Get Screen Information defined in DEMOInit()
    // This test is supposing height=480 (or near.)
    rmode = DEMOGetRenderModeObj();
    sc->screenWd = rmode->fbWidth;   // Screen Width
    sc->screenHt = rmode->efbHeight; // Screen Height

    // TMEM configuration
    for ( i = 0 ; i < NUM_REGIONS ; ++i )
    {
        // The region is used as a 32K cache.
        GXInitTexCacheRegion(
            &MyTexRegions[i],
            GX_FALSE,               // 32b mipmap
            0x00000 + i * 0x08000,  // tmem_even
            GX_TEXCACHE_32K,        // size_even
            0x80000 + i * 0x08000,  // tmem_odd
            GX_TEXCACHE_32K );      // size_odd
    }
    // User-defined tex region Callback
    GXSetTexRegionCallback(MyTexRegionCallback);

    // Load TPL file
    TEXGetPalette(&MyTplObj, "gxTests/tex-07.tpl");

    // This test doesn't use tpl data directly.
    // Just keeping pointers for image data and
    // copies them into buffers when used.
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        tdp = TEXGet(MyTplObj, i);
        sc->dataSource[i] = (u8*)tdp->textureHeader->data;
    }

    // Same parameter is used in every texture in tex-07.tpl.
    tdp = TEXGet(MyTplObj, 0);
    sc->dataSize = GXGetTexBufferSize(
                       tdp->textureHeader->width,
                       tdp->textureHeader->height,
                       tdp->textureHeader->format,
                       GX_FALSE,
                       0 );

    // Initialize texture objects
    for ( i = 0 ; i < NUM_TEXOBJS ; ++i )
    {
        sc->dataBuffer[i] = OSAlloc(sc->dataSize);
    
        GXInitTexObj(
            &sc->texture[i],
            (void*)sc->dataBuffer[i], // allocated buffer
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            (GXTexFmt)tdp->textureHeader->format,
            tdp->textureHeader->wrapS, // s
            tdp->textureHeader->wrapT, // t
            GX_FALSE ); // Mipmap is turned off.
        
        // UserData field keeps ID number.
        GXInitTexObjUserData(&sc->texture[i], (void*)i);
    }
    

    // Default scene control parameter settings

    // region invalidation switches
    for ( i = 0 ; i < NUM_ROWS - 2 ; ++i )
    {
        for ( j = 0 ; j < NUM_REGIONS ; ++j )
        {
            sc->invalidateSw[i][j] = 1;
        }
    }

    // cursor
    sc->curX = 0;
    sc->curY = 0;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mt, mv;
    s32  x, y;
    u16  token;

    // set projection to match screen space coordinate system
    DEMOSetupScrnSpc(sc->screenWd, sc->screenHt, 100.0F);
    
    // set up Zmode
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);
    
    // set up Tev for simple colored objects
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetNumChans(1);
    GXSetNumTexGens(0);

    // draw lines
    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    DrawLines();

    // set up Tev for textured objects
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetNumChans(0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    // reset token for synchronization
    token = 0;
    GXSetDrawSync(token);

    for ( y = 0 ; y < NUM_ROWS ; ++y )
    {
        // Sync is necessary because CPU modifies main
        // memory area for textures in run-time.
        while ( token != GXReadDrawSync() )
        {}
        
        // Copy source image into each buffer
        for ( x = 0 ; x < NUM_TEXOBJS ; ++x )
        {
            MyMemCopy(sc->dataSource[y], sc->dataBuffer[x], sc->dataSize);
        }

        // Invalidate cache regions
        if ( y == 0 || y == NUM_ROWS - 1 )
        {
            GXInvalidateTexAll();
        }
        else
        {
            for ( x = 0 ; x < NUM_REGIONS ; ++x )
            {
                if ( sc->invalidateSw[y-1][x] )
                {
                    GXInvalidateTexRegion(&MyTexRegions[x]);
                }
            }
        }
        
        // Draw textured quads for a row
        for ( x = 0 ; x < NUM_TEXOBJS ; ++x )
        {
            // modelview matrix
            MTXTrans(
                mt,
                (f32)ALI_COL(x),
                (f32)ALI_ROW(y),
                0.0F );
            GXLoadPosMtxImm(mt, GX_PNMTX0);
            
            GXLoadTexObj(&sc->texture[x], GX_TEXMAP0);
            DrawQuad();
        }
        
        // Send the next sync token
        GXSetDrawSync(++token);
    }

    // Captions
    MyDrawCaption(sc);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u16  down, dirs;
    
    // PAD
    down = DEMOPadGetButtonDown(0);
    dirs = DEMOPadGetDirsNew(0);

    // move cursor
    if ( dirs & DEMO_STICK_RIGHT )
    {
        sc->curX += 1;
    }
    if ( dirs & DEMO_STICK_LEFT )
    {
        sc->curX += NUM_REGIONS - 1;
    }
    if ( dirs & DEMO_STICK_DOWN )
    {
        sc->curY += 1;
    }
    if ( dirs & DEMO_STICK_UP )
    {
        sc->curY += NUM_ROWS - 3;
    }
    sc->curX %= NUM_REGIONS;
    sc->curY %= NUM_ROWS - 2;

    // toggle invalidation switch
    if ( down & PAD_BUTTON_A )
    {
        ++(sc->invalidateSw[sc->curY][sc->curX]);
        sc->invalidateSw[sc->curY][sc->curX] %= 2;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawQuad
    
    Description:    Draw a textured Quad. Each face can have differnt texture
                    which is specified by given texture number list.
    
    Arguments:      texNoList : a pointer to texture number list

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawQuad( void )
{
    u8   i;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);

    // draw the quad
    GXBegin(GX_QUADS, GX_VTXFMT1, 4);
        for ( i = 0 ; i < 4 ; ++i )
        {
            GXPosition1x8(i);
            GXTexCoord1x8(i);
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawLines
    
    Description:    Draw lines
    
    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLines( void )
{
    u8   i;

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    // draw lines
    GXBegin(GX_LINES, GX_VTXFMT1, NUM_REGIONS * 2);
        for ( i = 0 ; i < NUM_REGIONS ; ++i )
        {
            GXPosition3s16((s16)(i*72+64), 112, 0);
            GXColor1u16(0xFFFF); // White
            GXPosition3s16((s16)(i*72+64), 368, 0);
            GXColor1u16(0xFFFF); // White
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           MyDrawCaption
    
    Description:    Draw captions
    
    Arguments:      sc : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
static void MyDrawCaption( MySceneCtrlObj* sc )
{
    u32  i, j;

    DEMOInitCaption(DM_FT_OPQ, sc->screenWd, sc->screenHt);

    for ( i = 0 ; i < NUM_REGIONS ; ++i )
    {
        DEMOPrintf((s16)ALI_COL(i), (s16)(ALI_ROW(0)-16), 0, "[CACHE%d]", i);
        
        for ( j = 0 ; j < NUM_ROWS - 2 ; ++j )
        {
            DEMOPuts(
                (s16)(ALI_COL(i)+8),
                (s16)(ALI_ROW(j)+68),
                0,
                InvSwStr[sc->invalidateSw[j][i]]);
        }
    }
    
    // Cursor
    DEMOPrintf((s16)ALI_COL(sc->curX), (s16)(ALI_ROW(sc->curY)+68), 0, "%c", 0x7F);
    
    // "GXInvalidateTexAll"
    DEMOPrintf(ALI_COL(0), (s16)(ALI_ROW(3)+68), 0, "%s%s", InvTexAllStr, InvTexAllStr);
}

/*---------------------------------------------------------------------------*
    Name:           MyMemCopy
    
    Description:    Copy specified memory block and flush related cache.
                    
    Arguments:      src  : start address of source data
                    dst  : start address of destination
                    size : data size 
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void MyMemCopy( u8* src, u8* dst, u32 size )
{
    memcpy(dst, src, size);
    DCFlushRange(dst, size);
}

/*---------------------------------------------------------------------------*
    Name:           MyTexRegionCallback
    
    Description:    Tex cache allocator using simple round algorithm

    Arguments:      texObj : a pointer to texture object to be loaded
                    mapID  : destination texmap ID (just same as GXLoadTexObj)
    
    Returns:        appropriate tex cache region for loading texture.
 *---------------------------------------------------------------------------*/
static GXTexRegion* MyTexRegionCallback(GXTexObj* texObj, GXTexMapID mapID)
{
    #pragma unused(mapID)

    u32  texID, regionNum;
    
    texID     = (u32)GXGetTexObjUserData(texObj);
    regionNum = texID % NUM_REGIONS;

    return &MyTexRegions[regionNum];
}

/*---------------------------------------------------------------------------*
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n");
    OSReport("************************************************\n");
    OSReport("tex-invalid: tex cache region allocator test\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick : move the cursor\n");
    OSReport("A button   : toggle on/off invalidation switch\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
