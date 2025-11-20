/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gc-matrix-gc.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-matrix-gc.c $
    
    3     10/19/02 6:53p Hirose
    Changed location of data file.
    
    2     9/24/01 2:23p Hirose
    Changed flag definition.
    
    1     9/19/01 5:49p Carl
    Source files for GD matrix demo.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

#include <dolphin/gd.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

// This macro is used to copy a 3x4 matrix into a 3x3 matrix
#define COPY3x3(ms, md) \
    { md[0][0]=ms[0][0]; md[0][1]=ms[0][1]; md[0][2]=ms[0][2]; \
      md[1][0]=ms[1][0]; md[1][1]=ms[1][1]; md[1][2]=ms[1][2]; \
      md[2][0]=ms[2][0]; md[2][1]=ms[2][1]; md[2][2]=ms[2][2]; }

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main          ( void );
                          
static void CameraInit    ( void );
static void DrawInit      ( void );
static void DrawTick      ( void );
                          
static void AnimTick      ( void );
                          
static void ParameterInit ( void );

#ifdef LOAD_DL_FROM_FILE
static void LoadDLs       ( void );
#else                     
extern void CreateDLs     ( void );
#endif

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Display lists *************************************************************

// This set of display lists will each load an indexed position matrix
// and an indexed normal matrix, then draw one face of the cube.

GDLObj DrawDLOs[6];

/*---------------------------------------------------------------------------*/

// Actual primitive data follows

#define SIDE 30
#define NORM (sqrt(3.0))/3.0

// Remember:  Alignment of vertex arrays to 32B IS NOT required, but it
// may result in a slight performance improvement.

float FloatVert[] ATTRIBUTE_ALIGN(32) = 
{
    -SIDE,  SIDE, -SIDE,
    -SIDE,  SIDE,  SIDE,
    -SIDE, -SIDE,  SIDE,
    -SIDE, -SIDE, -SIDE,
     SIDE,  SIDE, -SIDE,
     SIDE, -SIDE, -SIDE,
     SIDE, -SIDE,  SIDE,
     SIDE,  SIDE,  SIDE,
};

float FloatNorm[] ATTRIBUTE_ALIGN(32) = 
{
    -1,  1, -1,
    -1,  1,  1,
    -1, -1,  1,
    -1, -1, -1,
     1,  1, -1,
     1, -1, -1,
     1, -1,  1,
     1,  1,  1,
};

float FloatTex[] ATTRIBUTE_ALIGN(32) =  
{
    0.0F, 0.0F, 
    1.0F, 0.0F,
    1.0F, 1.0F, 
    0.0F, 1.0F,
};

// This array is used to animate the Draw DL's.
// Since it is CPU-only data, no special treatment is needed.

float AnimData[6][3] =
{
    -1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f,  1.0f
};

// These matrices will be referenced by the GP from the Draw DL's.
// They could be ALIGNed, but they don't have to be.

Mtx posMats[6];
f32 nrmMats[6][3][3];

// Misc data...

Mtx v;          // view matrix
u32 rot;        // current cube rotation

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();         // Prepare the display lists and such

    ParameterInit();

    DEMOPadRead();      // Read the joystick for this frame

    // While the quit button is not pressed
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))   
    {           
        DEMOPadRead();  // Read the joystick for this frame

        AnimTick();     // Do animation based on input

        DEMOBeforeRender();
        
        DrawTick();     // Draw the model.
   
        DEMODoneRender();
    }

    OSHalt("End of test");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    
    Arguments:      v   view matrix to be passed to ViewInit
                    cameraLocScale  scale for the camera's distance from the 
                                    object - to be passed to ViewInit
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit      ( void )
{
    Mtx44 p;
    Vec camPt = {0.0F, 0.0F, 650.0F};
    Vec up = {0.0F, 1.0F, 0.0F};
    Vec origin = {0.0F, 0.0F, 0.0F};
    
    MTXFrustum(p, 112, -112, -160, 160, 500, 2000);

    GXSetProjection(p, GX_PERSPECTIVE);

    MTXLookAt(v, &camPt, &up, &origin); 
}


/*---------------------------------------------------------------------------*
    Name:           LoadDLs
    
    Description:    Loads the display lists used by the program from a file.
                    This routine is only called if LOAD_DL_FROM_FILE is defined.

    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
#ifdef LOAD_DL_FROM_FILE
static void LoadDLs ( void )
{
    s32 err;
    GDGList *DLDescArray;
    GDGList *PLDescArray;
    u32 numDLs, numPLs;
    u32 i;

    err = GDReadDLFile("gddemo/gdMatrix.gdl", &numDLs, &numPLs,
                       &DLDescArray, &PLDescArray);
    
    OSReport("(%d) Read %d DLs, %d PLs\n", err, numDLs, numPLs);

    ASSERTMSG(!err, "Error reading GDL file.\n");

    ASSERTMSG(numDLs >= 6 && numPLs >= 0, "Too little data in GDL file.\n");

    // Note: We put the DL length into the "offset" field, since that's
    // where the run-time code expects it.  We do this since the CreateDLs
    // function uses an oversize "length" field, and the actual valid data
    // length is saved in the "offset" field in that case.  Thus we do the
    // same thing here for consistency.

    for(i=0; i<6; i++)
    {
        GDInitGDLObj( &DrawDLOs[i],
                      DLDescArray[i].ptr,
                      DLDescArray[i].byteLength );
        GDSetCurrent( &DrawDLOs[i] );
        GDSetCurrOffset( DLDescArray[i].byteLength );
    }
    
    GDSetCurrent( NULL );
}
#endif


/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Calls the correct initialization function for the current 
                    model.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{
    u32 i;
    GXLightObj MyLight;
    GXColor White = {255, 255, 255, 255};

    // Create all the display lists.
    // In this demo, this is done at run-time.
    // We can also just load the DL's from a file created by a DL tool.
#ifdef LOAD_DL_FROM_FILE
    LoadDLs();
#else
    CreateDLs();
#endif

    CameraInit();   // Initialize the camera.

    GXClearVtxDesc();  // Set up VCD/VAT.

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    GXSetArray(GX_VA_TEX0, FloatTex, 8);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetArray(GX_VA_NRM, FloatNorm, 12);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc (GX_VA_POS, GX_INDEX8);
    GXSetArray(GX_VA_POS, FloatVert, 12);

    // Set up a light
    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, White);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);
    GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,   // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);

    // Set up TEV
    GXSetNumChans(1);
    GXSetNumTevStages(1);
    GXSetTevOp   (GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    // We'll send the face color in through TEVREG0
    GXSetTevColorIn(GX_TEVSTAGE0,
                    GX_CC_ZERO, GX_CC_RASC, GX_CC_C0, GX_CC_ZERO);

    // Fix up normals
    for(i = 0; i < 24; i++)
    {
        FloatNorm[i] *= NORM;
    }

    // Flush to memory
    DCFlushRange((void *)FloatNorm, sizeof(FloatNorm));

    // These matrices will be set up in DrawTick, below
    GXSetArray(GX_POS_MTX_ARRAY, posMats, sizeof(Mtx));
    GXSetArray(GX_NRM_MTX_ARRAY, nrmMats, sizeof(f32 [3][3]));
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the current model once.  
                    
    Arguments:      v       view matrix
                    m       model matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( void )
{   
    u32 i;
    static GXColor color[6] = {
        { 255,  0,  0, 255 },
        { 0,  255,  0, 255 },
        { 0,  0,  255, 255 },
        { 0, 255, 255, 255 },
        { 255, 0, 255, 255 },
        { 255, 255, 0, 255 }
    };
    
    // Draw the moving faces

    GXSetCullMode(GX_CULL_NONE);

    // Set the color, then call the DL to draw the geometry.
    for(i=0; i<6; i++)
    {
        GXSetTevColor(GX_TEVREG0, color[i]);
        GXCallDisplayList(GDGetGDLObjStart(&DrawDLOs[i]),
                          GDGetGDLObjOffset(&DrawDLOs[i]));
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Animates the camera and object based on the joystick's 
                    state.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick ( void )
{
    u32 i;
    Mtx ry, rz, mv, tm, mn;
    f32 tx, ty, tr;
    u16 buttons = DEMOPadGetButton(0);

    // Just simple controls right now...

    if(buttons & PAD_BUTTON_A)
    {
        // suspend animation
    } else {
    
        rot ++;
        if(rot > 2159)
            rot = 0;
    }

    // Set up our transformations...

    tx = 0.0f;
    ty = 0.0f;

    MTXRotDeg(ry, 'X', (float)rot);
    MTXRotDeg(rz, 'Y', (float)rot / 3.0F);
    MTXTrans(tm, tx, ty, 0);

    MTXConcat(rz, ry, mv);
    MTXConcat(tm, mv, mv);
    MTXConcat(v, mv, mv);

    MTXInverse(mv, mn);
    MTXTranspose(mn, mn);

    // Need to set up and flush all matrices for the AltDrawList's.
    if (rot % 120 < 60)
    {
        tr = rot % 60;
    } else {
        tr = 60 - (rot % 60);
    }

    // We recalculate and reload all the normal matrices, even though
    // they're actually the same for all faces.  This is just example
    // code to illustrate the various steps.
    for(i=0; i<6; i++)
    {
        MTXTrans(tm, AnimData[i][0]*(tr),
                     AnimData[i][1]*(tr),
                     AnimData[i][2]*(tr));
        MTXConcat(mv, tm, posMats[i]);
        COPY3x3(mn, nrmMats[i]);
    }

    // If you had a large number of matrices, you should flush them
    // as you calculate them.  Since we have just a few here, it's okay
    // to flush them all at once like this.
    DCFlushRange((void*) posMats, sizeof(posMats));
    DCFlushRange((void*) nrmMats, sizeof(nrmMats));
}

/*---------------------------------------------------------------------------*
    Name:           ParameterInit
    
    Description:    Initialize parameters for single frame display              
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void ParameterInit ( void )
{
    rot = 45;
}

/****************************************************************************/


