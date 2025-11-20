/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tev-ztex.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Tev/tev-ztex.c $
    
    5     5/13/01 2:48p Hirose
    removed obsolete flags
    
    4     10/04/00 4:30p Hirose
    A small bug fix + changed "MAC" flag to "EMU"
    
    3     5/13/00 12:42a Hirose
    adjusted parameters
    
    2     5/11/00 10:53p Hirose
    added interactive control / added codes for real HW
    
    1     5/10/00 7:30p Hirose
    initial version (only runnable for the MAC emulator)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    tev-ztex
        Z texture test
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <string.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
// Indices for textures stored in TPL file
#define TEX_GROUND      0
#define TEX_GROUND_Z    1
#define TEX_TREE        2
#define TEX_TREE_Z      3
#define TEX_WATER       4
#define NUM_TEXTURES    5

// Number of tree sprites
#define NUM_TREES       8

// Number of Z modes
#define NUM_ZMODES      4

// Max value of Z buffer
#define MAX_Z           0x00FFFFFF


#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Structure definitions
 *---------------------------------------------------------------------------*/
// for sprite control
typedef struct
{
    s16     xpos;   // left
    s16     ypos;   // bottom
    s16     zpos;
    s16     width;
    s16     height;
    s16     s0;
    s16     s1;
    s16     t0;
    s16     t1;
    u8      texIdx; // texture index
    u8      ztxIdx; // Z texture index
    u32     zbias;  // Z texture bias
} MySpriteObj;

// for entire scene control
typedef struct
{
    u32         cur;
    u32         zmode;
    s16         waterLv;
    MySpriteObj tree[NUM_TREES];
    u16         screenWd;
    u16         screenHt;
    GXTexObj    texture[NUM_TEXTURES];
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                    ( void );
static void DrawInit                ( MySceneCtrlObj* sc );
static void DrawTick                ( MySceneCtrlObj* sc );
static void AnimTick                ( MySceneCtrlObj* sc );
static void Draw3DSprite            ( MySpriteObj* sp, u32 zmode );
static void DrawWater               ( s16 level );
static void	MyTexInit               ( void );
static void SetShaderModeForSprites ( void );
static void SetShaderModeForWater   ( void );
static void	SetScreenSpace          ( u16 width, u16 height, f32 depth );
static void PrintIntro              ( void );

/*---------------------------------------------------------------------------*
   Data for drawing objects
 *---------------------------------------------------------------------------*/
static MySpriteObj GroundSprite =
{
    0, 0, 32767,        // xpos, ypos, zpos
    640, 256,           // width, height
    0x0000, 0x0280,     // s0=0.0, s1=2.5
    0x0000, 0x0100,     // t0=0.0, t1=1.0
    TEX_GROUND,         // texture index
    TEX_GROUND_Z,       // Z texture index
    0x00FF8000          // Z bias = -32768
};

static MySpriteObj DefaultTreeSprite =
{
    0, 0, 0,            // xpos, ypos, zpos (set later)
    128, 256,           // width, height
    0x0000, 0x0100,     // s0=0.0, s1=1.0
    0x0000, 0x0100,     // t0=0.0, t1=1.0
    TEX_TREE,           // texture index
    TEX_TREE_Z,         // Z texture index
    0x00FFFF80          // Z bias = -128
};

static s16 DefaultTreePos[NUM_TREES][3] =
{
    { -48, 192, 2688 },
    {   0,  48, 2370 },
    { 128, 168, 2716 },
    { 200,  56, 1900 },
    { 328, 120, 2464 },
    { 368, 120, 2484 },
    { 456, -16, 1780 },
    { 536,  96, 3096 },
};

static GXColor WaterBaseColor = { 160, 160, 192, 80 };
static GXColor BgColor        = { 120, 136, 152,  0 };

/*---------------------------------------------------------------------------*
   Data for Z texturing mode control
 *---------------------------------------------------------------------------*/
static GXZTexOp ZTexOpTbl[NUM_ZMODES] =
{
    GX_ZT_ADD, GX_ZT_ADD, GX_ZT_REPLACE, GX_ZT_DISABLE
};

static char* ZModeMsgTbl[NUM_ZMODES] =
{
    "Z texture add mode with bias",
    "Z texture add mode without bias",
    "Z texture replace mode without bias",
    "Z texture off"
};


/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj   SceneCtrl;                  // scene control parameters
static TEXPalettePtr    MyTplObj = NULL;            // texture palette
static GXTexObj         MyTexArray[NUM_TEXTURES];   // texture objects

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);  // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl); // Initialize vertex formats, pixel modes
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
    GXRenderModeObj*  rmode;
    u32               i;
    
    // Vertex Attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 8);

    // Get Screen Information defined in DEMOInit()
    rmode = DEMOGetRenderModeObj();
    sc->screenWd = rmode->fbWidth;   // Screen Width
    sc->screenHt = rmode->efbHeight; // Screen Height

    // Z compare mode
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_TRUE);
    GXSetZCompLoc(GX_FALSE);

    // Clear background by specified color at first
    // The operation can be done by dummy display copy.
    GXSetCopyClear(BgColor, MAX_Z);
    GXCopyDisp(DEMOGetCurrentBuffer(), GX_TRUE);
    

    // Load TPL file
    MyTexInit();
    

    // Default scene control parameter settings

    // Initialize position of tree sprites
    for ( i = 0 ; i < NUM_TREES ; ++i )
    {
        sc->tree[i] = DefaultTreeSprite;
        
        // Overwrite position for each tree
        sc->tree[i].xpos = DefaultTreePos[i][0];
        sc->tree[i].ypos = DefaultTreePos[i][1];
        sc->tree[i].zpos = DefaultTreePos[i][2];
    }

    // water level
    sc->waterLv = 176;

    // cursor
    sc->cur = 0;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    u32  i;

    // set projection to match screen space coordinate system
    SetScreenSpace(sc->screenWd, sc->screenHt, 0x01000000);

    // blend mode (semi-transparent values are blended)
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);

    //------------------------------------
    //  Draw sprite objects
    //------------------------------------
    SetShaderModeForSprites();

    // ground
    Draw3DSprite(&GroundSprite, sc->zmode);

    // trees
    for ( i = 0 ; i < NUM_TREES ; ++i )
    {
        Draw3DSprite(&sc->tree[i], sc->zmode);
    }

    //------------------------------------
    //  Draw water surface
    //------------------------------------
    
    // Z texture off
    GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
    
    SetShaderModeForWater();
    DrawWater(sc->waterLv);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    MySpriteObj* sp;
    u16          down;
    
    // PAD
    down = DEMOPadGetButtonDown(0);
    
    sp = &sc->tree[sc->cur];
    
    // Move selected tree sprite
    sp->xpos += ( DEMOPadGetStickX(0) / 16 );
    Clamp(sp->xpos, (s16)-128, (s16)sc->screenWd);
    sp->ypos += ( DEMOPadGetStickY(0) / 16 );
    Clamp(sp->ypos, (s16)-256, (s16)sc->screenHt);
    sp->zpos += ( DEMOPadGetSubStickY(0) / 4 );
    Clamp(sp->zpos, (s16)0, (s16)32767);
    
    // Select a tree
    if ( down & PAD_BUTTON_X )
    {
        sc->cur = ( sc->cur + 1 ) % NUM_TREES;
    }
    
    if ( down & PAD_BUTTON_Y )
    {
        sc->cur = ( sc->cur + NUM_TREES - 1 ) % NUM_TREES;
    }

    // Water level
    sc->waterLv += ( DEMOPadGetTriggerL(0) - DEMOPadGetTriggerR(0) ) / 32;
    Clamp(sc->waterLv, (s16)0, (s16)256);
    
    // Z texturing mode
    if ( down & PAD_BUTTON_B )
    {
        sc->zmode = ( sc->zmode + 1 ) % NUM_ZMODES;
        OSReport("%s\n", ZModeMsgTbl[sc->zmode]);
    }
}

/*---------------------------------------------------------------------------*
    Name:           Draw3DSprite
    
    Description:    Draw a 3D sprite specified by given data structure.
    
    Arguments:      sp    : a pointer to MySpriteObj structure
                    zmode : Z texturing mode

    Returns:        none
 *---------------------------------------------------------------------------*/
static void Draw3DSprite( MySpriteObj* sp, u32 zmode )
{
    s16  xpos1, ypos1;
    u32  zbias;

    // Z bias enable / disable
    zbias = ( zmode == 0 ) ? sp->zbias : 0U;

    // load textures and set up Z texturing
    GXLoadTexObj(&MyTexArray[sp->texIdx], GX_TEXMAP0);

#ifndef EMU // Z texturing is not available on the emulator
    GXLoadTexObj(&MyTexArray[sp->ztxIdx], GX_TEXMAP1);
    GXSetZTexture(
        ZTexOpTbl[zmode],
        GXGetTexObjFmt(&MyTexArray[sp->ztxIdx]),
        zbias );
#endif // EMU

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    xpos1 = (s16)(sp->xpos + sp->width);
    ypos1 = (s16)(sp->ypos + sp->height);

    // draw the quad
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16(sp->xpos, sp->ypos, sp->zpos);
        GXTexCoord2s16(sp->s0, sp->t1);
        GXPosition3s16(sp->xpos, ypos1,    sp->zpos);
        GXTexCoord2s16(sp->s0, sp->t0);
        GXPosition3s16(xpos1,    ypos1,    sp->zpos);
        GXTexCoord2s16(sp->s1, sp->t0);
        GXPosition3s16(xpos1,    sp->ypos, sp->zpos);
        GXTexCoord2s16(sp->s1, sp->t1);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawWater
        
    Description:    Draw water surface.
                    The water surface is one textured quad
                    which has some degree of gradient.

    Arguments:      level : water level

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawWater( s16 level )
{
    static s16  s0 = 0;
    s16  s1, lv1;

    s1  = (s16)(s0 + 0x1000);
    lv1 = (s16)(level - 256);

    // load texture
    GXLoadTexObj(&MyTexArray[TEX_WATER], GX_TEXMAP0);

    // set vertex descriptor
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // draw the quad
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3s16(0,   lv1, 0);
        GXTexCoord2s16(s0, 0x0000);
        GXPosition3s16(0,   level, 4096);
        GXTexCoord2s16(s0, 0x0600);
        GXPosition3s16(640, level, 4096);
        GXTexCoord2s16(s1, 0x0600);
        GXPosition3s16(640, lv1, 0);
        GXTexCoord2s16(s1, 0x0000);
    GXEnd();
    
    s0 = (s16)(++s0 % 0x100);
}

/*---------------------------------------------------------------------------*
    Name:           MyTexInit
    
    Description:    Load texture data from a TPL file and store it
                    into the texture object array. (TexArray)
                    
                    Since TPL file is not able to handle Z texture
                    format, Z textures are saved as other format
                    data. This function converts their format
                    and some other fields to make them suitable
                    for Z texturing.
    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void	MyTexInit( void )
{
    TEXDescriptorPtr  tdp;
    GXTexFmt          fmt;
    GXTexFilter       filter;
    u32  i;
    
    TEXGetPalette(&MyTplObj, "gxTests/tev-04.tpl");
    
    for ( i = 0 ; i < NUM_TEXTURES ; ++i )
    {
        tdp = TEXGet(MyTplObj, i);

        if ( i % 2 == 0 )
        {
            // Even number -> image texture
            fmt = (GXTexFmt)tdp->textureHeader->format;
            filter = GX_LINEAR;
            
            // Ground texture needs special wrap mode setting
            if ( i == TEX_GROUND )
            {
                tdp->textureHeader->wrapS = GX_REPEAT;
                tdp->textureHeader->wrapT = GX_CLAMP;
            }
        }
        else
        {
            // Odd number -> supporsed to be Z texture
            
            // format converstion
            switch(tdp->textureHeader->format)
            {
              case GX_TF_I8 :
                fmt = GX_TF_Z8;
                break;
              case GX_TF_IA8 :
                fmt = GX_TF_Z16;
                break;
              case GX_TF_RGBA8:
                fmt = GX_TF_Z24X8;
                break;
              default:
                OSHalt("Invalid data for Z texturing.");
            }
            
            filter = GX_NEAR;
        }
        
        GXInitTexObj(
            &MyTexArray[i],
            tdp->textureHeader->data,
            tdp->textureHeader->width,
            tdp->textureHeader->height,
            fmt,
            tdp->textureHeader->wrapS,
            tdp->textureHeader->wrapT,
            GX_FALSE ); // Mipmap is always off
        GXInitTexObjLOD(
            &MyTexArray[i],
            filter,     // Z doesn't work well with filtering
            filter,     // 
            0,
            0,
            0,
            GX_FALSE,
            GX_FALSE,
            GX_ANISO_1 );
    }
}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeForSprites
    
    Description:    Set up shader mode (TEV, color channel, etc.)
                    for drawing sprite objects.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetShaderModeForSprites( void )
{
    // accepts alpha > 128 which are used for blending
    GXSetAlphaCompare(GX_GREATER, 128, GX_AOP_AND, GX_ALWAYS, 0);

    // use no color channel
    GXSetNumChans(0);
    
    // use two textures that share the same texcoord
    // (it is possible only if the size is same.)
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    // TEV setting
#ifdef EMU
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
#else // !EMU  ( real hardware )
    GXSetNumTevStages(2);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE1, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
#endif // EMU

}

/*---------------------------------------------------------------------------*
    Name:           SetShaderModeForWater
    
    Description:    Set up shader mode (TEV, color channel, etc.)
                    for drawing water surface.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetShaderModeForWater( void )
{
    // through all alpha
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

    // use one color channel as constant color
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanMatColor(GX_COLOR0A0, WaterBaseColor);

    // use one texture
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    // TEV setting
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
}

/*---------------------------------------------------------------------------*
    Name:           SetScreenSpace
    
    Description:    Set up projection matrices to match the
                    screen coordinate system which is suitable to
                    display sprites.
                    The left-lower corner becomes (0, 0).
    
    Arguments:      width  : screen width
                    height : screen height
                    depth  : max depth
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void	SetScreenSpace( u16 width, u16 height, f32 depth )
{
    Mtx44  mp;
    Mtx    mv;

    // Orthographin projection
    MTXOrtho(mp, (f32)height, 0.0F, 0.0F, (f32)width, 0.0F, -depth);
    GXSetProjection(mp, GX_ORTHOGRAPHIC);
    
    // View matrix is set as identity.
    // (never changed through this demo)
    MTXIdentity(mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
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
    OSReport("tex-ztex: Z texture demo\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("B Button     : Change Z texturing mode\n");
    OSReport("Sticks       : Move selected tree sprite\n");
    OSReport("X/Y Buttons  : Toggle selection of a tree sprite\n");
    OSReport("L/R Triggers : Adjust water level\n");
    OSReport("************************************************\n\n");
}

/*============================================================================*/
