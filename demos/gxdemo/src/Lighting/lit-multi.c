/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-multi.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-multi.c $
    
    6     5/12/01 11:25p Hirose
    removed obsolete flags
    
    5     8/16/00 1:54a Hirose
    removed last workaround due to the library update
    
    4     8/08/00 6:53p Hirose
    added GXSetResetWritePipe call
    
    3     3/23/00 4:56p Hirose
    updated to use DEMOPad library
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    7     2/25/00 12:51a Hirose
    updated pad control functions to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     1/21/00 4:43p Hirose
    changed interactive control feature and single frame test
    
    4     1/20/00 10:02p Hirose
    updated to support 8 lights
    
    3     1/18/00 3:39p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    21    11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    20    11/12/99 4:29p Yasu
    Add GXSetNumTexGens(0)  for GX_PASSCLR
    
    19    11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    18    10/18/99 4:25p Hirose
    removed explicit casting to GXLightID
    
    17    10/07/99 3:31p Hirose
    changed camera portion / fixed some comments
    
    16    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    15    9/28/99 5:27p Hirose
    added singleframe test / fixed comments
    
    14    9/20/99 11:46a Ryan
    update to use new DEMO calls
    
    13    9/13/99 11:25p Hirose
    confirmed alignment/coherency
    removed all warnings
    
    12    9/02/99 5:36p Hirose
    
    11    8/31/99 4:56p Hirose
    turned all math functions into f32 version
    eliminated all tab characters
    
    10    8/28/99 9:39p Hirose
    added material control
    
    9     8/27/99 3:39p Yasu
    Add an explict cast to GXLightID 
    
    8     8/20/99 4:49p Hirose
    
    7     8/13/99 3:26p Hirose
    
    6     8/06/99 2:09p Hirose
    turned light position into camera space
    
    5     7/23/99 2:53p Ryan
    changed dolphinDemo.h to demo.h
    
    4     7/23/99 12:16p Ryan
    included dolphinDemo.h
    
    3     7/22/99 3:45p Hirose
    
    2     7/21/99 12:44p Hirose
    
    1     7/20/99 7:41p Hirose
    Created (Derived from lit-test00.c)
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   lit-multi
     Multiple lights in one RGB channel and light mask test
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI              3.14159265358979323846F
#define NUM_LIGHTS      8
#define NUM_MATERIALS   4

#define MODE_ANIMATION  1
#define MODE_MODELCTRL  2

/*---------------------------------------------------------------------------*
  Structure Definitions
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
    s32         v_theta;
    u32         color;
    u32         enable;
} MyLightObj;

// for entire scene control
typedef struct
{
    MyCameraObj cam;
    MyLightObj  lightCtrl[NUM_LIGHTS];
    u32         materialType;
    u32         modelType;
    Mtx         modelCtrl;
    u32         current;
    u32         mode;
} MySceneCtrlObj;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void DrawInit        ( MySceneCtrlObj* sc );
static void DrawTick        ( MySceneCtrlObj* sc );
static void AnimTick        ( MySceneCtrlObj* sc );
static void DrawModel       ( u32 model );
static void DrawAxes        ( void );
static void DrawLightMark   ( u8 ci, u8 type );
static void SetCamera       ( MyCameraObj* cam );
static void SetLight        ( MySceneCtrlObj* sc );
static void DisableLight    ( void );
static void PrintIntro      ( void );
static void StatusMessage   ( MySceneCtrlObj* sc );

/*---------------------------------------------------------------------------*
   Lighting Parameters
 *---------------------------------------------------------------------------*/
#define MAT_OFFSET     (NUM_LIGHTS * 2)
#define NUM_COLORS     (MAT_OFFSET + NUM_MATERIALS + 1)
#define REG_AMBIENT    MyColors[NUM_COLORS-1]

static GXColor MyColors[] ATTRIBUTE_ALIGN(32) =
{
    // for lights
    {0x90, 0x90, 0x90, 0xff},  // grey
    {0xb0, 0x00, 0x00, 0xff},  // red
    {0x00, 0xb0, 0x00, 0xff},  // green
    {0x00, 0x00, 0xb0, 0xff},  // blue
    {0x90, 0x90, 0x00, 0xff},  // yellow
    {0x00, 0x90, 0x90, 0xff},  // cyan
    {0x90, 0x00, 0x90, 0xff},  // magenta
    {0xa0, 0x50, 0x00, 0xff},  // orange

    {0x10, 0x10, 0x10, 0xff},  // dark grey
    {0x20, 0x00, 0x00, 0xff},  // dark red
    {0x00, 0x20, 0x00, 0xff},  // dark green
    {0x00, 0x00, 0x20, 0xff},  // dark blue
    {0x10, 0x10, 0x00, 0xff},  // dark yellow
    {0x00, 0x10, 0x10, 0xff},  // dark cyan
    {0x10, 0x00, 0x10, 0xff},  // dark magenta
    {0x20, 0x10, 0x00, 0xff},  // dark orange

    // for material
    {0xf0, 0xf0, 0xf0, 0xff},  // material(white)
    {0xf0, 0xf0, 0x60, 0xff},  // material(light-yellow)
    {0x60, 0xf0, 0xf0, 0xff},  // material(light-cyan)
    {0xf0, 0x60, 0xf0, 0xff},  // material(light-magenta)

    // for ambient
    {0x10, 0x10, 0x10, 0x00}   // ambient(almost black)
};

static s16 DefaultLightPos[NUM_LIGHTS][3] =
{
    // theta, phi, v_theta
    {   0,  60,  1   },
    {   0, -60,  358 },
    { 300,   0,  2   },
    {  60,   0,  359 },
    {  30, -30,  1   },
    { 330, -30,  358 },
    {  30,  30,  2   },
    { 330,  30,  359 }
};

static char* CtrlMsg[NUM_LIGHTS+1] =
{
    "White Light",
    "Red Light",
    "Green Light",
    "Blue Light",
    "Yellow Light",
    "Cyan Light",
    "Magenta Light",
    "Orange Light",
    "Model"
};

static char* MatName[NUM_MATERIALS] =
{
    "White",
    "Light Yellow",
    "Light Cyan",
    "Light Magenta"
};

static GXLightID LightIDTable[NUM_LIGHTS] =
{
    GX_LIGHT0,
    GX_LIGHT1,
    GX_LIGHT2,
    GX_LIGHT3,
    GX_LIGHT4,
    GX_LIGHT5,
    GX_LIGHT6,
    GX_LIGHT7
};
    
/*---------------------------------------------------------------------------*
   Model Data
 *---------------------------------------------------------------------------*/
#define SPHERE      0
#define CYLINDER    1
#define TORUS       2
#define ICOSA       3
#define DODECA      4
#define OCTA        5
#define CUBE        6

#define MODELS      7


static f32 ModelVertices[][3] ATTRIBUTE_ALIGN(32) =
{
    {   0.0F,   0.0F,   0.0F },
    {   0.0F,   0.0F, 495.0F },
    {   0.0F,   0.0F, 505.0F },
    {   0.0F,   5.0F, 500.0F },
    {   5.0F,   0.0F, 500.0F },
    {   0.0F,  -5.0F, 500.0F },
    {  -5.0F,   0.0F, 500.0F },
    {   0.0F,   0.0F, 475.0F },
    {   0.0F,   0.0F, 525.0F },
    {   0.0F,  25.0F, 500.0F },
    {  25.0F,   0.0F, 500.0F },
    {   0.0F, -25.0F, 500.0F },
    { -25.0F,   0.0F, 500.0F },
};

// For light direction mark
#define NUM_LM_EDGES  13
static u8 LightMarkEdge0[NUM_LM_EDGES*2] =
{
    0, 1, 1, 3, 1, 4, 1, 5, 1, 6, 2, 3, 2, 4,
    2, 5, 2, 6, 3, 4, 4, 5, 5, 6, 6, 3
};

static u8 LightMarkEdge1[NUM_LM_EDGES*2] =
{
    0, 7, 7, 9, 7, 10, 7, 11, 7, 12, 8, 9, 8, 10,
    8, 11, 8, 12, 9, 10, 10, 11, 11, 12, 12, 9
};

/*---------------------------------------------------------------------------*
   Camera configuration
 *---------------------------------------------------------------------------*/
static CameraConfig DefaultCamera =
{
    { 0.0F, 0.0F, 900.0F }, // location
    { 0.0F, 1.0F,   0.0F }, // up
    { 0.0F, 0.0F,   0.0F }, // tatget
    -160.0F, // left
    120.0F,  // top
    200.0F,  // near
    2000.0F  // far
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
static MySceneCtrlObj  SceneCtrl;    // scene control parameters

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main( void )
{
    DEMOInit(NULL);    // Init the OS, game pad, graphics and video.

    DrawInit(&SceneCtrl);       // Initialize veritex formats, scene and etc.

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
    
    Description:    Initializes the vertex attribute format, and sets
                    the array pointers and strides for the indexed data.
                    This function also sets up default scene parameters.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( MySceneCtrlObj* sc )
{
    u32 i;

    // set up a vertex attribute
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // set up array for indexed color access
    GXSetArray(GX_VA_CLR0, MyColors, 4*sizeof(u8));
    // set up array for indexed position
    GXSetArray(GX_VA_POS, ModelVertices, 3*sizeof(f32));


    // Default scene parameter settings

    // camera
    sc->cam.cfg = DefaultCamera;
    SetCamera(&sc->cam);   // never changes in this test 

    // set up light parameters
    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        sc->lightCtrl[i].theta   = DefaultLightPos[i][0];
        sc->lightCtrl[i].phi     = DefaultLightPos[i][1];
        sc->lightCtrl[i].v_theta = DefaultLightPos[i][2];
        sc->lightCtrl[i].color   = i;
        sc->lightCtrl[i].enable  = TRUE;
    }

    // model type, material type and current control number
    sc->modelType    = 0;
    sc->materialType = 0;
    sc->current      = 0;
    sc->mode         = MODE_ANIMATION;
    
    // initialize model rotation control matrix
    MTXScale(sc->modelCtrl, 250.0F, 250.0F, 250.0F);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the model by using given scene parameters 
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( MySceneCtrlObj* sc )
{
    Mtx  mr;  // Rotate matrix.
    Mtx  mv;  // Modelview matrix.
    Mtx  mvi; // Modelview matrix.
    u32  i;
    u8   t, ci;
    
    // Render mode = one color / no texture
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    
    // Disable lighting
    DisableLight();

    // Draw light direction marks
    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        MTXRotDeg(mr, 'y', sc->lightCtrl[i].theta);
        MTXConcat(sc->cam.view, mr, mv);
        MTXRotDeg(mr, 'x', - sc->lightCtrl[i].phi);
        MTXConcat(mv, mr, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);

        ci = (u8)(sc->lightCtrl[i].enable ? i : i + NUM_LIGHTS);
        t  = (u8)(( sc->current == i ) ? 1 : 0);
        DrawLightMark(ci, t);
    }
    
    // Modelview matrix for the model.
    MTXConcat(sc->cam.view, sc->modelCtrl, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // Axes are drawn if the model control mode is selected.
    if ( sc->mode & MODE_MODELCTRL )
    {
        DrawAxes();
    }

    // Set lighting parameters and draw a model
    SetLight(sc);
    DrawModel(sc->modelType);

}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters according to the pad status.
                    
    Arguments:      sc  : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( MySceneCtrlObj* sc )
{
    u16  down;
    Mtx  mrx, mry;
    s32  sx, sy;
    u32  i;
    u32  cur = sc->current;

    // PAD
    down = DEMOPadGetButtonDown(0);

    // Light Animation
    if ( sc->mode & MODE_ANIMATION )
    {
        for ( i = 0 ; i < NUM_LIGHTS ; ++i )
        {
            sc->lightCtrl[i].theta += sc->lightCtrl[i].v_theta;
            sc->lightCtrl[i].theta %= 360;
        }
    }

    // Animation ON/OFF
    if ( down & PAD_BUTTON_X )
    {
        sc->mode ^= MODE_ANIMATION;
    }


    // Model Rotation Control
    sx =   DEMOPadGetSubStickX(0) / 24;
    sy = - DEMOPadGetSubStickY(0) / 24;
    if ( sx == 0 && sy == 0 )
    {
        sc->mode &= ~MODE_MODELCTRL;
    }
    else
    {
        sc->mode |= MODE_MODELCTRL;
    }
    MTXRotDeg(mry, 'x', sy);
    MTXRotDeg(mrx, 'y', sx);
    MTXConcat(mry, sc->modelCtrl, sc->modelCtrl);
    MTXConcat(mrx, sc->modelCtrl, sc->modelCtrl);
        
    
    // Light Position Control
    sc->lightCtrl[cur].theta += ( DEMOPadGetStickX(0) / 24 );
    sc->lightCtrl[cur].theta = sc->lightCtrl[cur].theta % 360;
            
    sc->lightCtrl[cur].phi += ( DEMOPadGetStickY(0) / 24 );
    if ( sc->lightCtrl[cur].phi < -90 )
        sc->lightCtrl[cur].phi = -90;
    if ( sc->lightCtrl[cur].phi > 90 )
        sc->lightCtrl[cur].phi = 90;
    
    
    // Model Select
    if ( down & PAD_BUTTON_B )
    {
        sc->modelType = ( sc->modelType + 1 ) % MODELS;
    }

    // Light Enable / Disable
    if ( down & PAD_BUTTON_A )
    {
        if ( cur < NUM_LIGHTS )
        {
            sc->lightCtrl[cur].enable = !sc->lightCtrl[cur].enable;
            StatusMessage(sc);
        }
    }

    // Material Select
    if ( down & PAD_BUTTON_Y )
    {
         sc->materialType = ( sc->materialType + 1 ) % NUM_MATERIALS;
         OSReport("Material: %s\n", MatName[sc->materialType]);
    }

    // Light Select
    if ( down & PAD_TRIGGER_R )
    {
        sc->current = ( sc->current + 1 ) % NUM_LIGHTS;
        StatusMessage(sc);
    }
    if ( down & PAD_TRIGGER_L )
    {
        sc->current = ( sc->current + NUM_LIGHTS - 1 ) % NUM_LIGHTS;
        StatusMessage(sc);
    }

}

/*---------------------------------------------------------------------------*
    Name:           DrawModel
    
    Description:    Draws the specified model
                    
    Arguments:      model : specifies which model is to be displayed
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel( u32 model )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

    switch(model)
    {
        case CYLINDER :
            GXDrawCylinder(64);
            break;
        case TORUS :
            GXDrawTorus(0.375F, 12, 16);
            break;
        case SPHERE :
            GXDrawSphere(12, 24);
            break;
        case CUBE :
            GXDrawCube();
            break;
        case OCTA :
            GXDrawOctahedron();
            break;
        case ICOSA :
            GXDrawIcosahedron();
            break;
        case DODECA :
            GXDrawDodeca();
            break;
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawAxes
    
    Description:    Draws xyz-axes which shows model's direction
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawAxes( void )
{
    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GXBegin(GX_LINES, GX_VTXFMT0, 6);
        GXPosition3f32(0.0F, 0.0F, -1000.0F);
        GXColor4u8(96, 96, 96, 255);
        GXPosition3f32(0.0F, 0.0F, 1000.0F);
        GXColor4u8(96, 96, 96, 255);

        GXPosition3f32(0.0F, -1000.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);
        GXPosition3f32(0.0F, 1000.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);

        GXPosition3f32(-1000.0F, 0.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);
        GXPosition3f32(1000.0F, 0.0F, 0.0F);
        GXColor4u8(96, 96, 96, 255);
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           DrawLightMark
    
    Description:    Draws a mark which shows position of the light.
                    
    Arguments:      ci   : color index
                    type : type ( 0 or 1 )

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawLightMark( u8 ci, u8 type )
{
    u32  i;
    u8*  edgeTbl;

    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    
    if ( type == 0 )
    {
        edgeTbl = LightMarkEdge0;
    }
    else
    {
        edgeTbl = LightMarkEdge1;
    }

    GXBegin(GX_LINES, GX_VTXFMT0, NUM_LM_EDGES * 2);
        for ( i = 0 ; i < NUM_LM_EDGES * 2 ; ++i )
        {
            GXPosition1x8(edgeTbl[i]);
            GXColor1x8(ci);
        }
    GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           SetCamera
    
    Description:    sets view matrix and loads projection matrix into hardware
                    
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
    
    Description:    Sets up light & channel parameters
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void SetLight( MySceneCtrlObj* sc )
{
    GXLightID  id;
    f32        theta, phi;
    Vec        lpos;
    u32        i;
    u32        lmask = 0;
    

    for ( i = 0 ; i < NUM_LIGHTS ; ++i )
    {
        // Light Position
        theta  = (f32)sc->lightCtrl[i].theta * PI / 180.0F;
        phi    = (f32)sc->lightCtrl[i].phi   * PI / 180.0F;
        lpos.x = 500.0F * cosf(phi) * sinf(theta);
        lpos.y = 500.0F * sinf(phi);
        lpos.z = 500.0F * cosf(phi) * cosf(theta);

        // Convert light position into view space
        MTXMultVec(sc->cam.view, &lpos, &lpos);
    
        GXInitLightPos(&sc->lightCtrl[i].lobj, lpos.x, lpos.y, lpos.z);
        GXInitLightColor(
            &sc->lightCtrl[i].lobj,
            MyColors[sc->lightCtrl[i].color] );

        // Load light object into hardware
        id = LightIDTable[i];
        GXLoadLightObjImm(&sc->lightCtrl[i].lobj, id);
        
        // Light mask control
        lmask |= sc->lightCtrl[i].enable ? id : 0;
    }

    // light channel setting
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,     // enable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        lmask,         // light mask
        GX_DF_CLAMP,   // diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_DISABLE,    // disable channel
        GX_SRC_REG,    // amb source
        GX_SRC_REG,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_NONE,    // diffuse function
        GX_AF_NONE);
    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0A0, REG_AMBIENT);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, MyColors[sc->materialType + MAT_OFFSET]);
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
        GX_DISABLE,    // disable channel
        GX_SRC_VTX,    // amb source
        GX_SRC_VTX,    // mat source
        GX_LIGHT_NULL, // light mask
        GX_DF_NONE,    // diffuse function
        GX_AF_NONE );
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
    OSReport("lit-multi: multiple lights in one RGB channel\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Main Stick   : Move selected light\n");
    OSReport("Sub  Stick   : Rotate the model\n");
    OSReport("A Button     : Enable/Disable selected Light\n");
    OSReport("L/R Triggers : Select a light\n");
    OSReport("X Button     : Stop/Start light animation\n");
    OSReport("Y Button     : Change the material type\n");
    OSReport("B Button     : Change the Model\n");
    OSReport("************************************************\n");
    OSReport("\n\n");
}

/*---------------------------------------------------------------------------*
    Name:           StatusMessage
    
    Description:    Prints current status.
                    
    Arguments:      sc : pointer to the structure of scene control parameters
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void StatusMessage( MySceneCtrlObj* sc )
{
    OSReport("%s ", CtrlMsg[sc->current]);
    
    if ( sc->current < NUM_LIGHTS )
    {
        if ( sc->lightCtrl[sc->current].enable )
            OSReport("(Enabled)");
        else
            OSReport("(Disabled)");
    }
    OSReport("\n");
}

/*============================================================================*/
