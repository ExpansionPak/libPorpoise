/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-tx-alpha.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-tx-alpha.c $
    
    5     5/12/01 11:25p Hirose
    removed obsolete flags
    
    4     3/23/00 5:28p Hirose
    updated to use DEMOPad library
    
    3     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    7     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    6     2/25/00 3:59a Hirose
    updated pad control function to match actual HW1 gamepad
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    3     1/18/00 4:32p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    18    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    17    11/15/99 4:49p Yasu
    Change datafile name
    
    16    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    15    10/25/99 5:18p Hirose
    changed to use 16-bit index for drawing panels
    
    14    10/07/99 4:36p Hirose
    changed camera portion / fixed some comments
    
    13    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    12    9/30/99 4:29p Hirose
    
    11    9/27/99 11:37p Hirose
    added single frame test
    fixed some comments
    
    10    9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    9     9/13/99 4:43p Hirose
    confirmed alignment/coherency
    removed all warnings
    
    8     9/02/99 4:44p Hirose
    fixed camera setting / use GXInvalidateVtxCache()
    
    7     8/31/99 4:58p Hirose
    turned all math functions into f32 version
    eliminated all tab characters
    
    6     8/28/99 9:58p Hirose
    
    5     8/27/99 3:40p Yasu
    Change the parameter of GXSetBlendMode();
    
    4     8/27/99 2:45p Hirose
    added D/V cache coherency stuff
    
    3     8/26/99 2:44p Yasu
    Replace GXSetColorMode() to GXSetBlendMode().
    
    2     8/18/99 7:03p Hirose
    
    1     8/06/99 3:21p Hirose
    Created (copied lit-test03.c)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-tx-alpha
     Texture with lighting. One color channel/one alpha channel.
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
  Macro definitions
 *---------------------------------------------------------------------------*/
#define PI                3.14159265358979323846F
#define NUM_LIGHTS        2
#define NUM_PANELTYPE     6
#define PANEL_ARRAY_SIZE  ( 16 * 16 * 3 )

/*---------------------------------------------------------------------------*
  Structure definitions
 *---------------------------------------------------------------------------*/
// for camera
typedef struct
{
    Vec    location;
    Vec    up;
    Vec    target;
    f32    left;
    f32    top;
    f32    znear;
    f32    zfar;
} CameraConfig;

typedef struct
{
    CameraConfig  cfg; 
    Mtx           view;
    Mtx44         proj;
} MyCameraObj;

// for lighting
typedef struct
{
    GXLightObj  lobj;
    s32         theta;
    s32         phi;
    u32         color;
    u32         enable;
} MyLightObj;

// for model object
typedef struct
{
    u32         type;
    f32*        posArray;
    f32*        normArray;
} MyTessPanelObj;

// for entire scene control
typedef struct
{
    MyCameraObj     cam;
    MyLightObj      lightCtrl[NUM_LIGHTS];
    MyTessPanelObj  panel;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void CreateTessPanel ( MyTessPanelObj* pnl );
static void DrawTessPanel   ( MyTessPanelObj* pnl );
static void DrawBackGround  ( void );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightObj* lightCtrl );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
  Lighting and model parameters
 *---------------------------------------------------------------------------*/
#define REG_MATERIAL    MyColors[2]
#define REG_AMBIENT     MyColors[3]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0xff, 0xff, 0xff, 0x00},  // white
    {0x00, 0x00, 0x00, 0xff},  // alpha
    {0xff, 0xff, 0xff, 0xff},  // material
    {0x40, 0x40, 0x40, 0x40}   // ambient
};

static s16 DefaultLightPos[2][3] =
{
    {  150,  150, 300 },
    { -150, -150, 300 }
};

static char* CtrlMsg[] =
{
    "RGB Light",
    "Alpha Light",
};

#define ANGLE_ATN0   -39.0F
#define ANGLE_ATN1    40.0F
#define ANGLE_ATN2     0.0F

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 1200.0F }, // location
    { 0.0F, 1.0F, 0.0F },    // up
    { 0.0F, 0.0F, 0.0F },    // target
    -320.0F,  // left
    240.0F,   // top
    500.0F,   // near
    2000.0F   // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;
static TEXPalettePtr   MyTplObj = NULL;
static GXTexObj        MyTexObj0, MyTexObj1;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats, array
                                // and default scene settings

    PrintIntro();  // Print demo directions
         
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();
        DrawTick(&SceneCtrl);    // Draw the model.
        DEMODoneRender();
        DEMOPadRead();           // Update pad status.
        AnimTick(&SceneCtrl);    // Update animation.
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and allocates
                    the array memory for the indexed data.
                    This function also initializes scene control parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{
    u32 i;

    // vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    // textures from tpl
    TEXGetPalette(&MyTplObj, "gxTextrs.tpl");
    TEXGetGXTexObjFromPalette(MyTplObj, &MyTexObj0, 3);  // Water
    TEXGetGXTexObjFromPalette(MyTplObj, &MyTexObj1, 6);  // Floor board
    // invalidate data existing in texture cache
    GXInvalidateTexAll();


    // Scene control parameter settings
    // (This default is also used as single frame test.)

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // light parameters
    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        sc->lightCtrl[i].theta  = 0;
        sc->lightCtrl[i].phi    = 0;
        sc->lightCtrl[i].color  = i;
        sc->lightCtrl[i].enable = 1;
    }

    // allocate arrays
    sc->panel.posArray = OSAlloc(PANEL_ARRAY_SIZE * sizeof(f32));
    ASSERT( sc->panel.posArray != 0 );
    sc->panel.normArray = OSAlloc(PANEL_ARRAY_SIZE * sizeof(f32));
    ASSERT( sc->panel.normArray != 0 );
    sc->panel.type = 0;

    // create default panel
    CreateTessPanel(&sc->panel);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws models specified by given scene control parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  ms;  // Scale matrix.
    Mtx  mt;  // Translation matrix
    Mtx  mv;  // Modelview matrix.
    Mtx  mvi; // Modelview matrix.
    s32  ix, iy;

    //
    //  Draw the background wall
    //
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetNumTexGens(1);

    // Tev mode = no color / one texture
    GXSetNumTexGens(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetNumChans(0); // no color required
    
    // load texture
    GXLoadTexObj(&MyTexObj1, GX_TEXMAP0);
    // set up blend mode
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);

    MTXCopy(sc->cam.view, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    DrawBackGround();
    

    //
    //  Draw the front wall
    //

    // Tev mode = one color / one texture
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    SetLight(sc->lightCtrl);

    // load texture
    GXLoadTexObj(&MyTexObj0, GX_TEXMAP0);
    // set up blend mode
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);

    // draw 2x2=4 panels
    MTXScale(ms, 400.0F, 400.0F, 120.0F);
    for ( iy = 0 ; iy < 2 ; ++iy )
    {
        for ( ix = 0 ; ix < 2 ; ++ix )
        {
            MTXTrans(mt, (f32)(ix * 400 - 400), (f32)(iy * 400 - 400), 0);

            MTXConcat(sc->cam.view, mt, mv);
            MTXConcat(mv, ms, mv);

            GXLoadPosMtxImm(mv, GX_PNMTX0);
            MTXInverse(mv, mvi); 
            MTXTranspose(mvi, mv); 
            GXLoadNrmMtxImm(mv, GX_PNMTX0);
        
            DrawTessPanel(&sc->panel);
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene control parameters by using pad input.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters.

    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    static u32  cur     = 0;
    static u16  counter = 0;
    u32  i;
    u16  down;

    // PAD
    down = DEMOPadGetButtonDown(0);

    // Frame counter
    ++counter;
    
    // Light Blinking Control
    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        sc->lightCtrl[i].enable = 1;
    }
    if ( ( counter % 64 ) > 56 )
    {
        sc->lightCtrl[cur].enable = 0;  
    }

    // Light Position Calculation
    sc->lightCtrl[cur].theta += ( DEMOPadGetStickX(0) / 24 );
    if ( sc->lightCtrl[cur].theta < -90 )
        sc->lightCtrl[cur].theta = -90;
    if ( sc->lightCtrl[cur].theta > 90 )
        sc->lightCtrl[cur].theta = 90;
    
    sc->lightCtrl[cur].phi += ( DEMOPadGetStickY(0) / 24 );
    if ( sc->lightCtrl[cur].phi < -90 )
        sc->lightCtrl[cur].phi = -90;
    if ( sc->lightCtrl[cur].phi > 90 )
        sc->lightCtrl[cur].phi = 90;

    // Reset Light Angle
    if ( down & PAD_BUTTON_Y )
    {
        sc->lightCtrl[cur].theta = 0;
        sc->lightCtrl[cur].phi   = 0;
    }

    // Select Tesselated wall type
    if ( down & PAD_BUTTON_A )
    {
        sc->panel.type = ( sc->panel.type + 1 ) % NUM_PANELTYPE;
        CreateTessPanel(&sc->panel);
    }

    // Select Light
    if ( down & PAD_BUTTON_B )
    {
        cur = 1 - cur;
        OSReport("%s \n", CtrlMsg[cur]);
    }
}

/*---------------------------------------------------------------------------*
    Name:           CreateTessPanel
    
    Description:    Creates tesselated vertice data
                    
    Arguments:      pnl : pointer to a MyTessPanelObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CreateTessPanel( MyTessPanelObj* pnl )
{
    u8  ix, iy;
    u32 in;
    f32 px, py, pz, nx, ny, nz;
    f32 theta, phi, s;

    in = 0;

    for ( iy = 0 ; iy <= 15 ; ++iy )
    {
        py = (f32)iy / 15;
        
        for ( ix = 0 ; ix <= 15 ; ++ix )
        {
            px = (f32)ix / 15;
            
            switch(pnl->type)
            {
                case 0 :    // Z = 0
                {
                    pz = 0.0F;
                    nx = 0.0F;
                    ny = 0.0F;
                    nz = 1.0F;
                } break;
                case 1 :    // Z = sinX
                {
                    theta = (f32)ix * 2.0F * PI / 15.0F;
                    s = sqrtf( 1 + cosf(theta) * cosf(theta) );
                    pz = sinf(theta);
                    nx = - cosf(theta) / s;
                    ny = 0.0F;
                    nz = 1.0F / s;
                } break;
                case 2 :    // Z = sin2X
                {
                    theta = (f32)ix * 4.0F * PI / 15.0F;
                    s = sqrtf( 1.0F + cosf(theta) * cosf(theta) );
                    pz = sinf(theta);
                    nx = - cosf(theta) / s;
                    ny = 0.0F;
                    nz = 1.0F / s;
                } break;
                case 3 :    // Z = cos2X * sinY
                {
                    theta = (f32)ix * 4.0F * PI / 15.0F;
                    phi   = (f32)iy * 2.0F * PI / 15.0F;
                    s = sqrtf( 1.0F + cosf(theta) * cosf(theta) * sinf(phi) * sinf(phi) )
                      * sqrtf( 1.0F + sinf(theta) * sinf(theta) * cosf(phi) * cosf(phi) );
                    pz = cosf(theta) * cosf(phi);
                    nx = sinf(theta) * cosf(phi) / s; 
                    ny = cosf(theta) * sinf(phi) / s;
                    nz = 1.0F / s;
                } break;
                case 4 :    // Z = (X - 0.5)^2
                {
                    theta = (f32)ix / 15.0F - 0.5F;
                    pz = 4.0F * theta * theta;
                    s  = sqrtf( 1.0F + 64.0F * theta * theta );
                    nx = - (f32)theta * 8.0F / s;
                    ny = 0.0F;
                    nz = 1.0F / s;
                } break;
                case 5 :    // Z = - (Y - 0.5)^2
                {
                    theta = (f32)iy / 15.0F - 0.5F;
                    pz = 0.25F - 4.0F * theta * theta;
                    s  = sqrtf( 1.0F + 64.0F * theta * theta );
                    nx = 0.0F;
                    ny = (f32)theta * 8.0F / s;
                    nz = 1.0F / s;
                } break;
            }
            
            pnl->posArray[in]    = px;
            pnl->posArray[in+1]  = py;
            pnl->posArray[in+2]  = pz;
            pnl->normArray[in]   = nx;
            pnl->normArray[in+1] = ny;
            pnl->normArray[in+2] = nz;
        
            in += 3;
        }
    }

    // flushes all data into main memory from D-cache
    DCFlushRange(pnl->posArray,  PANEL_ARRAY_SIZE * sizeof(f32));
    DCFlushRange(pnl->normArray, PANEL_ARRAY_SIZE * sizeof(f32));
    // invalidates previous data existing in vertex cache
    GXInvalidateVtxCache();

}

/*---------------------------------------------------------------------------*
    Name:           DrawTessPanel
    
    Description:    Draws panel by using tesselated vertices
                    
    Arguments:      pnl : pointer to a MyTessPanelObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTessPanel( MyTessPanelObj* pnl )
{
    static f32 s = 0.0F;
    u32 i, x, y;
    f32 xt, yt0, yt1;

    // set up vertex descriptors
    // since GX_INDEX8 can handle up to 255 (not 256) vertices,
    // GX_INDEX16 is used here.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    
    // set up array pointers and sirides
    GXSetArray(GX_VA_POS, pnl->posArray,  3 * sizeof(f32));
    GXSetArray(GX_VA_NRM, pnl->normArray, 3 * sizeof(f32));

    // texture coord animation
    s += 0.005F;
    if ( s >= 1.0F )
    {
        s -= 1.0F;
    }
    
    // drawing loop
    for ( y = 0 ; y < 15 ; ++y )
    {
        i   = y * 16;
        yt0 = (f32)( y / 7.5F );
        yt1 = (f32)((y+1) / 7.5F );
        
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 32);
            for ( x = 0 ; x < 16 ; ++x )
            {
                xt = (f32)( x / 7.5F ) + s;
            
                GXPosition1x16((u16)i);
                GXNormal1x16((u16)i);
                GXTexCoord2f32(xt, yt0);
                GXPosition1x16((u16)(i+16));
                GXNormal1x16((u16)(i+16));
                GXTexCoord2f32(xt, yt1);
                ++i;
            }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawBackGround
    
    Description:    Draws a background wall
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawBackGround( void )
{
    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-500.0F, -500.0F, -150.0F);
        GXTexCoord2f32(   0.0F,    0.0F );
        GXPosition3f32(-500.0F,  500.0F, -150.0F);
        GXTexCoord2f32(   2.0F,    0.0F );
        GXPosition3f32( 500.0F,  500.0F, -150.0F);
        GXTexCoord2f32(   2.0F,    2.0F );
        GXPosition3f32( 500.0F, -500.0F, -150.0F);
        GXTexCoord2f32(   0.0F,    2.0F );
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    Sets view matrix and loads projection matrix into hardware
                    
    Arguments:      cam : pointer to the MyCameraObj structure
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetCamera( MyCameraObj* cam )
{
    MTXLookAt(
        cam->view,
        &cam->cfg.location,
        &cam->cfg.up,
        &cam->cfg.target );    

    MTXFrustum(
        cam->proj,
        cam->cfg.top,
        - (cam->cfg.top),
        cam->cfg.left,
        - (cam->cfg.left),
        cam->cfg.znear,
        cam->cfg.zfar );
    GXSetProjection(cam->proj, GX_PERSPECTIVE);
}

/*---------------------------------------------------------------------------*
    Name:           SetLight
    
    Description:    Sets light objects and color channels
                    
    Arguments:      lightCtrl : pointer to the array of MyLightObj structures
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( MyLightObj* lightCtrl )
{
    GXLightID rgbMask, alphaMask;
    f32 theta, phi, dx, dy, dz;
    s16 lx, ly, lz;
    u32 i;

    // set up light objects
    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        // direction is already calculated in the view space
        theta = (f32)lightCtrl[i].theta * PI / 180.0F;
        phi   = (f32)lightCtrl[i].phi   * PI / 180.0F;
        dx = cosf(phi) * sinf(theta);
        dy = sinf(phi);
        dz = - cosf(phi) * cosf(theta);

        // position is fixed and already in the view space
        lx = DefaultLightPos[i][0];
        ly = DefaultLightPos[i][1];
        lz = DefaultLightPos[i][2];

        GXInitLightDir(&lightCtrl[i].lobj, dx, dy, dz);
        GXInitLightPos(&lightCtrl[i].lobj, lx, ly, lz);
        GXInitLightColor(
            &lightCtrl[i].lobj,
            MyColors[lightCtrl[i].color] );
        GXInitLightAttn(
            &lightCtrl[i].lobj,
            ANGLE_ATN0,
            ANGLE_ATN1,
            ANGLE_ATN2,
            1.0F,
            0.0F,
            0.0F );
    }

    // loads each light object
    GXLoadLightObjImm(&lightCtrl[0].lobj, GX_LIGHT0);
    GXLoadLightObjImm(&lightCtrl[1].lobj, GX_LIGHT1);

    // light mask control
    rgbMask   = ( lightCtrl[0].enable ) ? GX_LIGHT0 : (GXLightID)0;
    alphaMask = ( lightCtrl[1].enable ) ? GX_LIGHT1 : (GXLightID)0;

    // color channel setting
    GXSetNumChans(1); // number of active color channels
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        rgbMask,     // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_SPOT);
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_ENABLE,   // disable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        alphaMask,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_SPOT);
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, REG_MATERIAL);

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
    OSReport("******************************************************\n");
    OSReport("lit-tx-alpha: one RGB and one Alpha light with texture\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Stick     : Move selected Light\n");
    OSReport("B Button  : Select Light\n");
    OSReport("A Button  : Change front wall type\n");
    OSReport("******************************************************\n");
    OSReport("\n");
}

/*============================================================================*/
