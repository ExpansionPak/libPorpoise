/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-spot.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-spot.c $
    
    6     5/12/01 11:25p Hirose
    removed obsolete flags
    
    5     6/12/00 4:34p Hirose
    reconstructed DEMOPad library
    
    4     5/12/00 6:09p Hirose
    fix due to GXInitLight*v macro definition change
    
    3     3/23/00 5:28p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    10    3/03/00 4:21p Alligator
    integrate with ArtX source
    
    9     2/26/00 8:34p Hirose
    changed pad control function to match actual HW1 gamepad
    
    8     2/25/00 4:04a Hirose
    updated pad control functions (temporary)
    
    7     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    6     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    5     1/25/00 2:54p Carl
    Changed to standardized end of test message
    
    4     1/18/00 3:39p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    3     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     1/12/00 6:48p Hirose
    updated sample coefficient values for new formula with non-squared a2
    
    25    11/17/99 4:33p Hirose
    changed to use GXInitLightPosv/Dirv macros
    
    24    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    23    11/12/99 4:29p Yasu
    Add GXSetNumTexGens(0)  for GX_PASSCLR
    
    22    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    21    10/25/99 5:18p Hirose
    changed to use 16-bit index for drawing panel
    
    20    10/18/99 4:26p Hirose
    removed explicit casting to GXLightID
    used MTXMultVecSR to convert light direction
    
    19    10/05/99 3:28p Hirose
    fixed light direction transform matrix
    
    18    10/02/99 11:43p Hirose
    added camera control
    
    17    10/02/99 10:18p Hirose
    added some features
    
    16    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    15    9/30/99 4:28p Hirose
    added single frame test
    
    14    9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    13    9/13/99 11:23p Hirose
    confirmed alignment/coherency
    removed all warnings
    
    12    9/02/99 4:57p Hirose
    fixed camera setting / used GXInvalidateVtxCache()
    
    11    8/31/99 4:57p Hirose
    turned all math functions into f32 version
    eliminated all tab characters
    
    10    8/28/99 12:06a Hirose
    added D/V cache coherency functions
    improved DrawTessPanel()
    
    9     8/27/99 4:21p Yasu
    Add explicit cast to GXLightID to avoid warning message
    
    8     8/23/99 5:29p Hirose
    
    7     7/23/99 5:40p Hirose
    
    6     7/23/99 2:53p Ryan
    changed dolphinDemo.h to demo.h
    
    5     7/23/99 12:16p Ryan
    included dolphinDemo.h
    
    4     7/22/99 4:45p Hirose
    
    3     7/21/99 11:34p Hirose
    
    2     7/21/99 12:42p Hirose
    
    1     7/20/99 7:42p Hirose
    Created (Simply copied lit-test01.c)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-spot
     Spot light angle attenuation test
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
#define NUM_LIGHTS        4
#define NUM_PANELTYPE     6
#define PANEL_ARRAY_SIZE  ( 16 * 16 * 3 )

#define Clamp(val,min,max) \
    ((val) = (((val)<(min)) ? (min) : ((val)>(max)) ? (max) : (val)))

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
    f32         a0;
    f32         a1;
    f32         a2;
} AngleAttn;

typedef struct
{
    GXLightObj  lobj;
    Vec         pos;
    s32         theta;
    s32         phi;
    u32         colorIdx;
    u32         attnType;
    u32         enable;
} MyLightObj;

typedef struct
{
    MyLightObj  light[NUM_LIGHTS];
    GXAttnFn    attnFunc;
} MyLightEnvObj;

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
    MyLightEnvObj   lightEnv;
    MyTessPanelObj  panel;
    u32             cur;
    u32             mode;
    u32             mark;
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
static void DrawLightMark   ( MyLightObj* light );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MyLightEnvObj* le, Mtx view );
static void DisableLight    ( void );
static void PrintIntro      ( void );
static void StatusMessage   ( MyLightObj* light );

/*---------------------------------------------------------------------------*
  Lighting and model parameters
 *---------------------------------------------------------------------------*/
#define BLACK        MyColors[4]
#define REG_MATERIAL MyColors[5]
#define REG_AMBIENT  MyColors[6]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    {0xff, 0xff, 0xff, 0xff},  // white
    {0xff, 0x00, 0x00, 0xff},  // red
    {0x00, 0xff, 0x00, 0xff},  // green
    {0x00, 0x00, 0xff, 0xff},  // blue

    {0x00, 0x00, 0x00, 0x00},  // black
    {0xc0, 0xc0, 0xc0, 0xff},  // material
    {0x10, 0x10, 0x10, 0xff}   // ambient
};

static Vec DefaultLightPos[4] =
{
    {  200,  200, 450 },
    {  200, -200, 450 },
    { -200, -200, 450 },
    { -200,  200, 450 }
};

#define NUM_OF_ATTNS    20
static AngleAttn LightAttnSamples[NUM_OF_ATTNS] =
{
    {  -9.0F,  10.0F,   0.0F  },
    { -24.0F,  25.0F,   0.0F  },
    { -49.0F,  50.0F,   0.0F  },
    {  -4.0F,   5.0F,   0.0F  },
    { -18.5F,  20.0F,   0.0F  },
    { -47.0F,  50.0F,   0.0F  },
    {-299.0F, 200.0F, 100.0F  },
    { -74.0F,  50.0F,  25.0F  },
    { -26.0F,  18.0F,   9.0F  },
    {  -2.0F,   2.0F,   1.0F  },
    {-297.0F, 200.0F, 100.0F  },
    { -72.5F,  50.0F,  25.0F  },
    {   0.5F,   0.5F,   0.25F },
    {  -0.5F,   2.0F,   1.0F  },
    {   1.0F,  -2.0F,   1.0F  },
    {  38.5F, -40.0F,   0.0F  },
    { -17.0F,  48.0F, -32.0F  },
    {  -3.0F,   8.0F,  -4.0F  },
    {   0.0F,   1.0F,   0.0F  },
    {   0.5F,   0.0F,   0.0F  }
};

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
   Strings for messages
 *---------------------------------------------------------------------------*/
static char* CtrlMsg[] =
{
    "White Light",
    "Red Light",
    "Green Light",
    "Blue Light"
};

static char* ModeMsg[] =
{
    "Direction & attenuation control",
    "Position control",
    "Camera control"
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
    
    DrawInit(&SceneCtrl);       // Initialize vertex formats, array
                                // and default scene settings

    PrintIntro(); // Print demo directions
         
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


    // Default scene control parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;

    // light parameters
    for ( i = 0 ; i < 4 ; ++i )
    {
        sc->lightEnv.light[i].pos      = DefaultLightPos[i];
        sc->lightEnv.light[i].theta    = 0;
        sc->lightEnv.light[i].phi      = 0;
        sc->lightEnv.light[i].attnType = 0;
        sc->lightEnv.light[i].enable   = TRUE;
        sc->lightEnv.light[i].colorIdx = i;
    }
    sc->lightEnv.attnFunc = GX_AF_SPOT;

    // allocate arrays
    sc->panel.posArray = OSAlloc(PANEL_ARRAY_SIZE * sizeof(f32));
    ASSERT( sc->panel.posArray != 0 );
    sc->panel.normArray = OSAlloc(PANEL_ARRAY_SIZE * sizeof(f32));
    ASSERT( sc->panel.normArray != 0 );
    sc->panel.type = 0;

    // create default panel
    CreateTessPanel(&sc->panel);
    
    //  current control object and control mode
    sc->cur  = 0;
    sc->mode = 0;
    sc->mark = 1;
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw models specified by given scene control parameters.
                    
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
   
    // render mode = one color / no texture
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    
    // set camera
    SetCamera(&sc->cam);

    // set lighting
    SetLight(&sc->lightEnv, sc->cam.view);

    // draw 2x2=4 panels as a wall
    MTXScale(ms, 400.0F, 400.0F, 120.0F);
    for ( iy = 0 ; iy < 2 ; ++iy )
    {
        for ( ix = 0 ; ix < 2 ; ++ix )
        {
            MTXTrans(mt, ix * 400.0F - 400.0F, iy * 400.0F - 400.0F, 0.0F);

            MTXConcat(sc->cam.view, mt, mv);
            MTXConcat(mv, ms, mv);

            GXLoadPosMtxImm(mv, GX_PNMTX0);
            MTXInverse(mv, mvi); 
            MTXTranspose(mvi, mv); 
            GXLoadNrmMtxImm(mv, GX_PNMTX0);
        
            DrawTessPanel(&sc->panel);
        }
    }

    // turn off lighting
    DisableLight();

    // draw a light mark
    MTXCopy(sc->cam.view, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    if ( sc->mark == 1 && sc->cur < NUM_LIGHTS )
    {
        DrawLightMark(&sc->lightEnv.light[sc->cur]);
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
    static u16  counter = 0;
    u32  i;
    MyLightObj* lo = sc->lightEnv.light;


    // Frame counter
    ++counter;
    
    // Light Blinking Control
    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        lo[i].enable = TRUE;
    }

    if ( ( counter % 32 ) > 28 )
    {
        lo[sc->cur].enable = FALSE;
    }

    // Each light control mode
    if ( sc->cur < NUM_LIGHTS )
    {
        // Direction
        lo[sc->cur].theta += ( DEMOPadGetStickX(0) / 32 );
        Clamp(lo[sc->cur].theta, -60, 60);
        lo[sc->cur].phi += ( DEMOPadGetStickY(0) / 32 );
        Clamp(lo[sc->cur].phi, -60, 60);
        
        // Position X/Y
        lo[sc->cur].pos.x += ( DEMOPadGetSubStickX(0) / 16 );
        Clamp(lo[sc->cur].pos.x, -320, 320);
        lo[sc->cur].pos.y += ( DEMOPadGetSubStickY(0) / 16 );
        Clamp(lo[sc->cur].pos.y, -240, 240);
        
        // Position Z
        lo[sc->cur].pos.z += ( DEMOPadGetTriggerR(0) / 16 )
                           - ( DEMOPadGetTriggerL(0) / 16 );
        Clamp(lo[sc->cur].pos.z, 50, 600);
        
        // Attenuation parameter change
        if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_X )
        {
            lo[sc->cur].attnType =
                ( lo[sc->cur].attnType + 1 ) % NUM_OF_ATTNS;
            StatusMessage(&lo[sc->cur]);
        }
        if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_Y )
        {
            lo[sc->cur].attnType =
                ( lo[sc->cur].attnType + NUM_OF_ATTNS - 1 ) % NUM_OF_ATTNS;
            StatusMessage(&lo[sc->cur]);
        }
    }
    // General / camera control mode
    else
    {
        // Camera calculation
        Vec* cloc = &sc->cam.cfg.location;
        
        cloc->x += ( DEMOPadGetStickX(0) / 16 ) * 2;
        Clamp(cloc->x, -480, 480);
        
        cloc->y += ( DEMOPadGetStickY(0) / 16 ) * 2;
        Clamp(cloc->y, -320, 320);
        
        cloc->z =
            sqrtf( 1200 * 1200 - cloc->y * cloc->y - cloc->x * cloc->x );


        // Spotlight attenuation ON/OFF
        if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_X )
        {
            sc->lightEnv.attnFunc = GX_AF_NONE;
            OSReport("GX_AF_NONE\n");
        }
        if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_Y )
        {
            sc->lightEnv.attnFunc = GX_AF_SPOT;
            OSReport("GX_AF_SPOT\n");
        }
        
        // Tesselated wall type Select
        if ( DEMOPadGetButtonDown(0) & PAD_TRIGGER_R )
        {
            sc->panel.type += 1;
            sc->panel.type %= NUM_PANELTYPE;
            CreateTessPanel(&sc->panel);
        }
        if ( DEMOPadGetButtonDown(0) & PAD_TRIGGER_L )
        {
            sc->panel.type += NUM_PANELTYPE - 1;
            sc->panel.type %= NUM_PANELTYPE;
            CreateTessPanel(&sc->panel);
        }
    }


    // Hide light mark
    sc->mark = (u32)(( DEMOPadGetButton(0) & PAD_BUTTON_A ) ? 0 : 1);

    // Select light / camera mode
    if ( DEMOPadGetButtonDown(0) & PAD_BUTTON_B )
    {
        sc->cur = ( sc->cur + 1 ) % ( NUM_LIGHTS + 1 );
        
        if ( sc->cur < NUM_LIGHTS )
        {
            StatusMessage(&sc->lightEnv.light[sc->cur]);
        }
        else
        {
            OSReport("Camera mode\n");
        }
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
        py = (f32)iy / 15.0F;
        
        for ( ix = 0 ; ix <= 15 ; ++ix )
        {
            px = (f32)ix / 15.0F;
            
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
                    s = sqrtf( 1.0F + cosf(theta) * cosf(theta) );
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
    u32 i, x, y;

    // set up vertex descriptors
    // since GX_INDEX8 can handle up to 255 (not 256) vertices,
    // GX_INDEX16 is used here.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);

    // set up array pointers and sirides
    GXSetArray(GX_VA_POS, pnl->posArray,  3 * sizeof(f32));
    GXSetArray(GX_VA_NRM, pnl->normArray, 3 * sizeof(f32));

    for ( y = 0 ; y < 15 ; ++y )
    {
        i = y * 16;

        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 32);
            for ( x = 0 ; x < 16 ; ++x )
            {
                GXPosition1x16((u16)i);
                GXNormal1x16((u16)i);
                GXPosition1x16((u16)(i+16));
                GXNormal1x16((u16)(i+16));
                ++i;
            }
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawLightMark
    
    Description:    Draws a mark which shows light position and direction.
                    
    Arguments:      light : pointer to a MyLightObj structure
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLightMark( MyLightObj* light )
{
    f32 theta, phi, len;
    Vec ldir;

    theta = (f32)light->theta * PI / 180.0F;
    phi   = (f32)light->phi   * PI / 180.0F;
    ldir.x = sinf(theta);
    ldir.y = sinf(phi) * cosf(theta);
    ldir.z = cosf(phi) * cosf(theta);

    len = fabsf(light->pos.z / ldir.z);
    ldir.x = light->pos.x + ldir.x * len;
    ldir.y = light->pos.y + ldir.y * len;
    ldir.z = 0;

    // set up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);

    // set up array pointers and sirides
    GXSetArray(GX_VA_CLR0, MyColors, 4 * sizeof(u8));

    GXBegin(GX_LINES, GX_VTXFMT0, 8);
        GXPosition3f32(light->pos.x, light->pos.y, 700.0F);
        GXColor1x8((u8)light->colorIdx);
        GXPosition3f32(light->pos.x, light->pos.y, 0.0F);
        GXColor1x8((u8)light->colorIdx);

        GXPosition3f32(light->pos.x, -480.0F, light->pos.z);
        GXColor1x8((u8)light->colorIdx);
        GXPosition3f32(light->pos.x,  480.0F, light->pos.z);
        GXColor1x8((u8)light->colorIdx);

        GXPosition3f32(-640.0F, light->pos.y, light->pos.z);
        GXColor1x8((u8)light->colorIdx);
        GXPosition3f32( 640.0F, light->pos.y, light->pos.z);
        GXColor1x8((u8)light->colorIdx);

        GXPosition3f32(light->pos.x, light->pos.y, light->pos.z);
        GXColor1x8((u8)light->colorIdx);
        GXPosition3f32(ldir.x, ldir.y, ldir.z);
        GXColor1x8((u8)light->colorIdx);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    set view matrix and load projection matrix into hardware
                    
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
                    
    Arguments:      le   : pointer to a MyLightEnvObj structure
                    view : view matrix.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void SetLight( MyLightEnvObj* le, Mtx view )
{
    f32 theta, phi;
    Vec ldir, lpos;
    AngleAttn attn;
    u32 i;
    u32 lmask = 0;

    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        // direction
        theta = (f32)le->light[i].theta * PI / 180.0F;
        phi   = (f32)le->light[i].phi   * PI / 180.0F;
        ldir.x = sinf(theta);
        ldir.y = sinf(phi) * cosf(theta);
        ldir.z = - cosf(phi) * cosf(theta);
        
        // convert direction into view space
        MTXMultVecSR(view, &ldir, &ldir);
        
        // convert position into view space
        lpos = le->light[i].pos;
        MTXMultVec(view, &lpos, &lpos);
        
        // angular attenuation parameters
        attn = LightAttnSamples[le->light[i].attnType];

        // set parameters for one light
        GXInitLightDirv(&le->light[i].lobj, &ldir);
        GXInitLightPosv(&le->light[i].lobj, &lpos);
        GXInitLightColor(&le->light[i].lobj, MyColors[le->light[i].colorIdx]);
        GXInitLightAttn(
            &le->light[i].lobj,
            attn.a0,
            attn.a1,
            attn.a2,
            1.0F,
            0.0F,
            0.0F );
    }

    // loads each light object
    GXLoadLightObjImm(&le->light[0].lobj, GX_LIGHT0);
    GXLoadLightObjImm(&le->light[1].lobj, GX_LIGHT1);
    GXLoadLightObjImm(&le->light[2].lobj, GX_LIGHT2);
    GXLoadLightObjImm(&le->light[3].lobj, GX_LIGHT3);

    // light mask control
    lmask = le->light[0].enable ? ( lmask | GX_LIGHT0 ) : lmask;
    lmask = le->light[1].enable ? ( lmask | GX_LIGHT1 ) : lmask;
    lmask = le->light[2].enable ? ( lmask | GX_LIGHT2 ) : lmask;
    lmask = le->light[3].enable ? ( lmask | GX_LIGHT3 ) : lmask;

    // channel setting
    GXSetNumChans(1);     // number of active color channels
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,        // enable channel
        GX_SRC_REG,       // amb source
        GX_SRC_REG,       // mat source
        lmask,            // light mask
        GX_DF_CLAMP,      // diffuse function
        le->attnFunc);    // attenuation function
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_DISABLE,       // disable channel
        GX_SRC_REG,       // amb source
        GX_SRC_REG,       // mat source
        0,                // light mask
        GX_DF_NONE,       // diffuse function
        GX_AF_NONE);      // attenuation function
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, REG_MATERIAL);
}

/*---------------------------------------------------------------------------*
    Name:           DisableLight
    
    Description:    Disables lighting
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DisableLight( void )
{
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0,
        GX_DISABLE,  // disable channel
        GX_SRC_VTX,  // amb source
        GX_SRC_VTX,  // mat source
        0,           // light mask
        GX_DF_NONE,  // diffuse function
        GX_AF_NONE);
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
    OSReport("lit-spot: spot light angle attenuation test\n");
    OSReport("******************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("B Button     : Select a light or camera\n");
    OSReport("    [each light control mode]\n");
    OSReport("Main Stick   : Change direction of selected light\n");
    OSReport("Sub Stick    : Change XY-position of selected light\n");
    OSReport("L/R Triggers : Change Z-position of selected light\n");
    OSReport("X/Y Buttons  : Change the spotlight shape\n");
    OSReport("    [camera control mode]\n");
    OSReport("Main Stick   : Move the camera location\n");
    OSReport("X/Y Buttons  : Enable/Disable spot function\n");
    OSReport("L/R Triggers : Change the wall shape\n");
    OSReport("******************************************************\n");
    OSReport("\n");
}

/*---------------------------------------------------------------------------*
    Name:           StatusMessage
    
    Description:    Prints current status.
 *---------------------------------------------------------------------------*/
static void StatusMessage(MyLightObj* light)
{
    u32 i;
    OSReport("%s ", CtrlMsg[light->colorIdx]);
    
    i = light->attnType;
    OSReport(" a0 = %f ", LightAttnSamples[i].a0);
    OSReport(" a1 = %f ", LightAttnSamples[i].a1);
    OSReport(" a2 = %f ", LightAttnSamples[i].a2);

    OSReport("\n");
}

/*============================================================================*/
