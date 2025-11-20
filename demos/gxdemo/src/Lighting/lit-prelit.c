/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-prelit.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-prelit.c $
    
    6     7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    5     5/12/01 11:25p Hirose
    removed obsolete flags
    
    4     5/12/00 6:09p Hirose
    fix due to GXInitLight*v macro definition change
    
    3     3/23/00 5:28p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    7     2/25/00 3:59a Hirose
    updated pad control function to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    4     1/19/00 7:05p Alligator
    fix EPPC errors
    
    3     1/18/00 4:32p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    17    11/17/99 4:33p Hirose
    changed to use GXInitLightPosv/Dirv macros
    
    16    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    15    11/12/99 4:29p Yasu
    Add GXSetNumTexGens(0)  for GX_PASSCLR
    
    14    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    13    10/18/99 4:27p Hirose
    removed explicit casting to GXLightID
    used MTXMultVecSR to convert light direction
    
    12    10/07/99 4:36p Hirose
    changed camera portion / fixed some comments
    
    11    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    10    9/27/99 8:00p Hirose
    added single frame test
    
    9     9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    8     9/13/99 4:23p Hirose
    confirmed alignment/coherency
    avoided all warnings
    
    7     9/02/99 4:44p Hirose
    fixed camera setting / used GXInvalidateVtxCache()
    
    6     8/31/99 4:58p Hirose
    eliminated all tab characters
    
    5     8/28/99 12:34a Yasu
    Append explicit casting to GXLightID
    
    4     8/27/99 10:24p Hirose
    added D/V cache coherency stuff
    added channel source select
    
    3     8/23/99 6:48p Hirose
    
    2     8/19/99 8:17p Hirose
    
    1     8/06/99 3:21p Hirose
    Created (copied lit-test03.c)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-prelit
     Vertex lighting with a prelit color
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI               3.14159265358979323846F

#define NUM_FAKELIGHTS   8
#define WALL_SIZE        64
#define WALL_ARRAY_SIZE  ( WALL_SIZE * WALL_SIZE * 3 )

#define ALL_LIGHTS   \
    ( GX_LIGHT0 | GX_LIGHT1 | GX_LIGHT2 | GX_LIGHT3 )

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
    s32         velocity;
    u32         color;
} MyLightObj;

// for pre-lighting
typedef struct
{
    f32  xpos;
    f32  ypos;
    f32  zpos;
    f32  vx;
    f32  vy;
    u32  color;
} MyFakeLightObj;

// for entire scene control
typedef struct
{
    MyCameraObj    cam;
    MyFakeLightObj fakeLight[NUM_FAKELIGHTS];
    u8*            preLitColor;
    MyLightObj     lightCtrl[4];
    GXColorSrc     ambSrc;
    GXColorSrc     matSrc;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawWall        ( void );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MySceneCtrlObj* sc );
static void PreLighting     ( MySceneCtrlObj* sc );
static void PrintIntro      ( void );
static void StatusMessage   ( MySceneCtrlObj* sc );

/*---------------------------------------------------------------------------*
  Model and Lighting data
 *---------------------------------------------------------------------------*/
#define REG_MATERIAL     MyColors[12]
#define REG_AMBIENT      MyColors[13]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0xc0, 0xc0, 0xc0, 0xff},  // white
    {0xc0, 0xc0, 0x00, 0xff},  // yellow
    {0x00, 0xc0, 0xc0, 0xff},  // cyan
    {0xc0, 0x00, 0xc0, 0xff},  // magenta

    {0xc0, 0x00, 0x00, 0xff},  // red
    {0x00, 0xc0, 0x00, 0xff},  // green
    {0x00, 0x00, 0xc0, 0xff},  // blue
    {0xc0, 0x60, 0x00, 0xff},  // orange
    {0xc0, 0x60, 0x90, 0xff},  // pink
    {0x70, 0xc0, 0x40, 0xff},  // olive green
    {0xc0, 0xa0, 0x70, 0xff},  // beige
    {0x40, 0x00, 0xc0, 0xff},  // indigo

    {0xe0, 0xe0, 0xe0, 0xff},  // material reg.
    {0x40, 0x40, 0x40, 0xff},  // ambient reg.
};

static f32 FixedNormal[] ATTRIBUTE_ALIGN(32) =
{ 0.0F, 0.0F, 1.0F };

static s16 LightPos[4][3] =
{
    {  400,  400, 400 },
    {  400, -400, 400 },
    { -400, -400, 400 },
    { -400,  400, 400 }
};

static Vec LightRotAxis[4] =
{
    { -1.0F,   1.0F, 0.0F },
    {  1.0F,   1.0F, 0.0F },
    {  1.0F,  -1.0F, 0.0F },
    { -1.0F,  -1.0F, 0.0F }
};

static MyFakeLightObj DefaultFakeLightStatus[NUM_FAKELIGHTS] =
{
    // xpos  ypos   zpos   vx     vy    color
    { 24.0F, 24.0F, 4.0F,  0.4F,  0.8F, 4  },
    { 24.0F, 40.0F, 4.5F, -0.6F,  0.2F, 5  },  
    { 40.0F, 24.0F, 5.0F, -0.5F,  0.4F, 6  },
    { 40.0F, 40.0F, 5.5F, -0.2F, -0.7F, 7  }, 
    { 32.0F,  8.0F, 6.0F,  0.3F,  0.5F, 8  },
    {  8.0F, 32.0F, 6.5F, -0.8F, -0.2F, 9  },  
    { 56.0F, 32.0F, 7.0F,  0.4F,  0.4F, 10 },
    { 32.0F, 56.0F, 7.5F, -0.5F,  0.0F, 11 }
};

#define ANGLE_ATN0   -99.0F
#define ANGLE_ATN1   100.0F
#define ANGLE_ATN2     0.0F

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 1200.0F }, // location
    { 0.0F, 1.0F,    0.0F }, // up
    { 0.0F, 0.0F,    0.0F }, // tatget
    -320.0F, // left
    240.0F,  // top
    500.0F,  // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
  Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj SceneCtrl;    // scene control parameters

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);    // Initialize the OS, game pad, graphics and video.
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats, arrays, lights, etc.

    PrintIntro();               // Print demo directions

    StatusMessage(&SceneCtrl);      // Print current status
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
    
    Description:    Initializes the vertex attribute, the array pointers
                    and strides for the indexed data and lighting parameters.
                    This function also set up modelview matrix.
                    
    Arguments:      sc : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{ 
    Mtx  ms, mt, mv, mvi;
    f32  scaling;
    u32  i;

    // sets vertex attributes
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_U8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);

    // sets array pointers
    sc->preLitColor = OSAlloc( WALL_ARRAY_SIZE );
    ASSERTMSG(sc->preLitColor != 0, "Memory allocation failed.");

    GXSetArray(GX_VA_CLR0, sc->preLitColor, 3*sizeof(u8));
    GXSetArray(GX_VA_NRM, FixedNormal, 3*sizeof(f32));


    // initialize scene control parameters

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // set hardware light parameters
    for ( i = 0 ; i < 4 ; ++i )
    {
        sc->lightCtrl[i].theta    = 70;
        sc->lightCtrl[i].velocity = 2;
        sc->lightCtrl[i].color    = i;
    }

    // copy default fake light parameters
    for ( i = 0 ; i < NUM_FAKELIGHTS ; ++i )
    {
        sc->fakeLight[i] = DefaultFakeLightStatus[i];
    }

    // channel color source control
    sc->ambSrc = GX_SRC_VTX;
    sc->matSrc = GX_SRC_REG;


    // modelview matrix is never changed in this test.
     scaling = 800.0F / WALL_SIZE;
    MTXScale(ms, scaling, scaling, 1.0F);  // scale
    MTXTrans(mt, -400.0F, -400.0F, 0.0F);  // translation
    MTXConcat(sc->cam.view, mt, mv);
    MTXConcat(mv, ms, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once. 
                    GXInit makes GX_PNMTX0 the default matrix.
                    
    Arguments:      sc : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    // render mode = one color / no texture
    GXSetNumTexGens(0);
	GXSetNumChans(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // pre-lighting
    PreLighting(sc);

    // hardware lights and channel control
    SetLight(sc);

    // sets preloaded modelview matrix and draws wall
    GXSetCurrentMtx(GX_PNMTX0);
    DrawWall();
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Moves light objects and controls scene by using pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u16  down = DEMOPadGetButtonDown(0);
    u32  i;

    // changes channel source setting
    if ( down & PAD_BUTTON_A )
    {
        sc->ambSrc = ( sc->ambSrc == GX_SRC_REG ) ? GX_SRC_VTX : GX_SRC_REG;
        
        if ( sc->ambSrc == GX_SRC_REG )
        {
            sc->matSrc = ( sc->matSrc == GX_SRC_REG ) ? GX_SRC_VTX : GX_SRC_REG;
        }
        
        StatusMessage(sc);
    }

    // moves fake lights
    for ( i = 0 ; i < NUM_FAKELIGHTS ; ++i )
    {
        sc->fakeLight[i].xpos += sc->fakeLight[i].vx;
        sc->fakeLight[i].ypos += sc->fakeLight[i].vy;
        
        if ( sc->fakeLight[i].xpos < 0.0F || sc->fakeLight[i].xpos > WALL_SIZE )
        {
            sc->fakeLight[i].vx = - sc->fakeLight[i].vx;
        }
        if ( sc->fakeLight[i].ypos < 0.0F || sc->fakeLight[i].ypos > WALL_SIZE )
        {
            sc->fakeLight[i].vy = - sc->fakeLight[i].vy;
        }
    }

    // moves hardware lights
    for ( i = 0 ; i < 4 ; ++i )
    {
        sc->lightCtrl[i].theta += sc->lightCtrl[i].velocity;
        if ( sc->lightCtrl[i].theta < 0 || sc->lightCtrl[i].theta > 90 )
        {
            sc->lightCtrl[i].velocity = - sc->lightCtrl[i].velocity;
        }    
    }

}

/*---------------------------------------------------------------------------*
    Name:           DrawWall
    
    Description:    Draws a tesselated wall
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawWall( void )
{
    u32 x, y, i;

    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    
    for ( y = 0 ; y < WALL_SIZE - 1 ; ++y )
    {
        i = y * WALL_SIZE;
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, WALL_SIZE * 2);
            for ( x = 0 ; x < WALL_SIZE ; ++x )
            {
                GXPosition3u8((u8)x, (u8)y, 0);
                GXNormal1x8(0);
                GXColor1x16((u16)(i+x));
                GXPosition3u8((u8)x, (u8)(y+1), 0);
                GXNormal1x8(0);
                GXColor1x16((u16)(i+x+WALL_SIZE));
            }
        GXEnd();
    }
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
    
    Description:    Sets hardware light objects and color channels
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetLight( MySceneCtrlObj* sc )
{
    Mtx mr;
    u32 i;

    for ( i = 0 ; i < 4 ; ++i )
    {
        Vec ldir = { 0.0F, 0.0F, -1.0F };
        Vec lpos, laxis;
    
        // Light direction
        laxis = LightRotAxis[i];
        MTXRotAxisDeg(mr, &laxis, (f32)sc->lightCtrl[i].theta);
        MTXMultVec(mr, &ldir, &ldir);
        // convert direction into view space
        MTXMultVecSR(sc->cam.view, &ldir, &ldir);
        
        // Light position
        lpos.x = (f32)LightPos[i][0];
        lpos.y = (f32)LightPos[i][1];
        lpos.z = (f32)LightPos[i][2];
        // convert position into view space
        MTXMultVec(sc->cam.view, &lpos, &lpos);

        GXInitLightDirv(&sc->lightCtrl[i].lobj, &ldir);
        GXInitLightPosv(&sc->lightCtrl[i].lobj, &lpos);
        GXInitLightAttn(
            &sc->lightCtrl[i].lobj,
            ANGLE_ATN0,
            ANGLE_ATN1,
            ANGLE_ATN2,
            1.0F,
            0.0F,
            0.0F );
        GXInitLightColor(
            &sc->lightCtrl[i].lobj,
            MyColors[sc->lightCtrl[i].color] );
    }

    // loads each light object
    GXLoadLightObjImm(&sc->lightCtrl[0].lobj, GX_LIGHT0);
    GXLoadLightObjImm(&sc->lightCtrl[1].lobj, GX_LIGHT1);
    GXLoadLightObjImm(&sc->lightCtrl[2].lobj, GX_LIGHT2);
    GXLoadLightObjImm(&sc->lightCtrl[3].lobj, GX_LIGHT3);

    // color channel setting
    GXSetNumChans(1); // number of color channels
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,   // enable channel
        sc->ambSrc,  // amb source
        sc->matSrc,  // mat source
        ALL_LIGHTS,  // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_SPOT);
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_DISABLE,  // disable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        0,           // light mask
        GX_DF_NONE,  // diffuse function
        GX_AF_NONE);
    // set up material and ambient register
    GXSetChanMatColor(GX_COLOR0A0, REG_MATERIAL);
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT);

}

/*---------------------------------------------------------------------------*
    Name:           PreLighting
    
    Description:    Creates an array of pre-lighted vertex color by
                    using fake lights.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PreLighting( MySceneCtrlObj* sc )
{
    f32 x, y, z, r, g, b, dx, dy, bri;
    u32 iv, il, col;

    // calculates colors on each vertex
    iv = 0;
    for ( y = 0.0F ; y < (f32)WALL_SIZE ; y += 1.0F )
    {
        for ( x = 0.0F ; x < (f32)WALL_SIZE ; x += 1.0F )
        {
            r = g = b = 0.0F;
            
            for ( il = 0 ; il < NUM_FAKELIGHTS ; ++il )
            {
                col = sc->fakeLight[il].color;
                dx  = sc->fakeLight[il].xpos - x;
                dy  = sc->fakeLight[il].ypos - y;
                z   = sc->fakeLight[il].zpos * sc->fakeLight[il].zpos;
                
                bri = z / ( dx * dx + dy * dy + z );
                r += bri * MyColors[col].r;
                g += bri * MyColors[col].g;
                b += bri * MyColors[col].b;
            }
            
            sc->preLitColor[iv]   = (u8)(( r > 255.0F ) ? 255 : r);
            sc->preLitColor[iv+1] = (u8)(( g > 255.0F ) ? 255 : g);
            sc->preLitColor[iv+2] = (u8)(( b > 255.0F ) ? 255 : b);
            iv += 3;
        }
    }

    // flushes all data existing in D-cache into main memory
    DCFlushRange(sc->preLitColor, WALL_ARRAY_SIZE);
    // invalidates previous data existing in vertex cache
    GXInvalidateVtxCache();
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
    OSReport("*****************************************\n");
    OSReport("lit-prelit: prelit vertex lighting test\n");
    OSReport("*****************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("A Button     : Change channel sources\n");
    OSReport("*****************************************\n");
    OSReport("\n");
}

/*---------------------------------------------------------------------------*
    Name:           StatusMessage
    
    Description:    Prints the current status of scene control.

    Arguments:      sc : pointer to the structure of scene control parameters

    Returns:        none
 *---------------------------------------------------------------------------*/
static void StatusMessage( MySceneCtrlObj* sc )
{
    if ( sc->ambSrc == GX_SRC_VTX )
        OSReport("Ambient:Vertex  ");
    else
        OSReport("Ambient:Register");

    if ( sc->matSrc == GX_SRC_VTX )
        OSReport("  Material:Vertex  ");
    else
        OSReport("  Material:Register");

    OSReport("\n");
}

/*============================================================================*/
