/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-light-gc.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-light-gc.c $
    
    3     10/19/02 6:53p Hirose
    Changed location of data file.
    
    2     10/03/01 11:37a Hirose
    Deleted redundant ifdef.
    
    1     9/24/01 2:17p Hirose
    Initial checki in.
  
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   gd-light
     Displaylist demo with lighting commands
     [Main source code for GAMECUBE exectable]
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>
#include <dolphin/gd.h>

#include "gd-light.h"

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI          3.14159265358979323846F
#define PI_2        (PI*2)

#define Clamp(val,min,max) \
    ((val) = (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val)))

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main                ( void );

static void CreateModelVtxArray ( void );
static void PatchLightStatus    ( void );
static void PrepareDL           ( void );

static void CameraInit          ( Mtx v );
static void DrawInit            ( void );
static void DrawTick            ( void );
static void AnimTick            ( void );
static void PrintIntro          ( void );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Model display list
static u8*      ModelDL;
static u32      ModelDLSize;
static u32*     ModelDLPatchList;
// Model geometry
static s16*     VtxPosArray = NULL;
static s16*     VtxNrmArray = NULL;

// Model/Camera matrices
static Mtx      ViewMtx;
static Mtx      ModelMtx;

// Light position control
static Vec      Light0Pos;
static Vec      Light1Dir;
static s32      LightTheta, LightPhi;


/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL); // Init the OS, game pad, graphics and video.
    DrawInit();     // Initialize vertex formats and array pointers.

    PrintIntro();   // Print demo directions

    while(!( DEMOPadGetButton(0) & PAD_BUTTON_MENU ))
    {
		DEMOBeforeRender();
        
        DrawTick();  // Draw the model.
        
        DEMODoneRender();
        
        DEMOPadRead();
        AnimTick();
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Name:           CreateModelVtxArray
    
    Description:    Create indexed data array for the model
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CreateModelVtxArray( void )
{
    u32  size, cnt_p, cnt_n;
    s32  i, j;
    f32  fs, ft, fr;

    // allocate necessary array memories
    if ( VtxPosArray != NULL )
        OSFree(VtxPosArray);
    if ( VtxNrmArray != NULL )
        OSFree(VtxNrmArray);

    size = MODEL_N0 * MODEL_N1 * 3;

    VtxPosArray = (s16*)OSAlloc(size * sizeof(s16));
    VtxNrmArray = (s16*)OSAlloc(size * sizeof(s16));

    // make array data
    cnt_p = cnt_n = 0;
    for ( i = 0 ; i < MODEL_N0 ; ++i )
    {
        for ( j = 0 ; j < MODEL_N1; ++j )
        {
            // Position
            fs = i * PI_2 / MODEL_N0;
            ft = j * PI_2 / MODEL_N1;
            fr = 1.0F + MODEL_R * cosf(fs);

            VtxPosArray[cnt_p++] = (s16)(fr * cosf(ft) * SCALE_Q);
            VtxPosArray[cnt_p++] = (s16)(fr * sinf(ft) * SCALE_Q);
            VtxPosArray[cnt_p++] = (s16)(MODEL_R * sinf(fs) * SCALE_Q);

            // Normal
            VtxNrmArray[cnt_n++] = (s16)(cosf(ft) * cosf(fs) * SCALE_Q);
            VtxNrmArray[cnt_n++] = (s16)(sinf(ft) * cosf(fs) * SCALE_Q);
            VtxNrmArray[cnt_n++] = (s16)(sinf(fs) * SCALE_Q);
        }
    }

    // make sure data is written to main memory
    DCFlushRange(VtxPosArray, size * sizeof(s16));
    DCFlushRange(VtxNrmArray, size * sizeof(s16));

}

/*---------------------------------------------------------------------------*
    Name:           PrepareDL
    
    Description:    [create mode] Allocate memory for display list and call
                    the function CreateModelDL to create the display list.
                    [load mode] Load GDL file from the disc.

    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrepareDL ( void )
{
#ifdef  LOAD_DL_FROM_FILE
    //---------------------------------------------------------
    // File mode : Read pre-generated GDL file from file
    //---------------------------------------------------------
    s32         err;
    GDGList*    dlArray;
    GDGList*    plArray;
    u32         nDls, nPls;
    
    err = GDReadDLFile("gddemo/gdLight.gdl", &nDls, &nPls, &dlArray, &plArray);
    
    // This demo only expects 1DL/1PL.
    ASSERTMSG( ( nDls == 1 && nPls == 1 ),
               "This data doesn't match requirement of this demo.\n" );

    ModelDL     = dlArray[0].ptr;
    ModelDLSize = dlArray[0].byteLength;
    
    // Check length of the patch list.
    ASSERTMSG( ( plArray[0].byteLength == MODELDL_NUM_PATCHES * sizeof(u32) ),
               "Length of the patch list doesn't match requirement of this demo.\n" );
               
    ModelDLPatchList = plArray[0].ptr;

#else
    //---------------------------------------------------------
    // Create mode : Create display list in this application
    //---------------------------------------------------------
    
    // Allocate memory for the DL.
    ModelDL = OSAlloc(MODELDL_SIZE_MAX);
    ASSERTMSG(ModelDL, "Memory allocation failed.\n");
    ModelDLPatchList = (u32*)OSAlloc(MODELDL_NUM_PATCHES * sizeof(u32));
    ASSERTMSG(ModelDLPatchList, "Memory allocation failed.\n");
    
    ModelDLSize = CreateModelDL(ModelDL, ModelDLPatchList);
    OSReport("Size = %d\n", ModelDLSize);

#endif
}

/*---------------------------------------------------------------------------*
    Name:           PatchLightStatus
    
    Description:    Patches lighting status in the model display list.
                    Position, direction informations are overrided in run-time.
                    Locations are given by the patch list.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PatchLightStatus( void )
{
    Vec         lpos;
    GDLObj      dlObj;

    // Prepare DL object fot patching.
    GDInitGDLObj(&dlObj, ModelDL, ModelDLSize);
    GDSetCurrent(&dlObj);

    // Patch LIGHT0(Pos)
    lpos = Light0Pos;
    MTXMultVec(ViewMtx, &lpos, &lpos);
    GDSetCurrOffset(ModelDLPatchList[MODELDL_PATCH_LIGHT0POS]);
    GDSetLightPos(GX_LIGHT0, lpos.x, lpos.y, lpos.z);
    
    // Patch LIGHT1(Dir)
    lpos = Light1Dir;
    MTXMultVecSR(ViewMtx, &lpos, &lpos);
    GDSetCurrOffset(ModelDLPatchList[MODELDL_PATCH_LIGHT1DIR]);
    GDSetSpecularDir(GX_LIGHT1, lpos.x, lpos.y, lpos.z);

    GDFlushCurrToMem();
    GDSetCurrent(NULL); // release DL object
}

/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.
                    Initialize the view matrix.
                    
    Arguments:      v      view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit ( Mtx v )
{
    Mtx44   p;      // projection matrix
    Vec     up      = { 0.0F, 1.0F, 0.0F };
    Vec     camLoc  = { 0.0F, 0.0F, 6.0F };
    Vec     objPt   = { 0.0F, 0.0F, 0.0F };
    f32     left    = -0.050F;
    f32     top     = 0.0375F;
    f32     znear   = 0.1F;
    f32     zfar    = 10.0F;
    
    MTXFrustum(p, top, -top, left, -left, znear, zfar);
    GXSetProjection(p, GX_PERSPECTIVE);
    
    MTXLookAt(v, &camLoc, &up, &objPt);    
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and
                    array pointers.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{ 
    // prepare display list
    PrepareDL();
    // create model data geometries
    CreateModelVtxArray();

    // model rotation and light location
    MTXIdentity(ModelMtx);
    LightTheta = 30;
    LightPhi   = 30;

    // camera
    CameraInit(ViewMtx);

    // set up vertex attributes
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, QUANTIZE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, QUANTIZE);

    // set up array pointers for indexed lookup
    GXSetArray(GX_VA_POS, VtxPosArray, 3*sizeof(s16));
    GXSetArray(GX_VA_NRM, VtxNrmArray, 3*sizeof(s16));

    //--------------------------------------------------
    //  Rendering pipe setting
    //--------------------------------------------------
    GXSetNumTexGens(0);
    GXSetNumTevStages(2);
    GXSetNumChans(2);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);

    // Output = Color channel 0 + Color channel 1
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_RASC,
                                  GX_CC_ONE,  GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the lit model
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( void )
{
    Mtx     mv, mvi;

    //--------------------------------------------------
    //  Lighting parameters
    //--------------------------------------------------
    PatchLightStatus();

    //--------------------------------------------------
    //  Geometry transformation
    //--------------------------------------------------
    // model has a rotation about y axis
    MTXConcat(ViewMtx, ModelMtx, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInvXpose(mv, mvi);
    GXLoadNrmMtxImm(mvi, GX_PNMTX0);

    //--------------------------------------------------
    //  Draw the model
    //--------------------------------------------------

    // Call the display list
    GXCallDisplayList(ModelDL, ModelDLSize);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Controls model rotation.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( void )
{
    Mtx  mrx, mry;
    f32  f_theta, f_phi;

    // Model Rotation Calculation
    MTXRotDeg(mry, 'x', -(DEMOPadGetStickY(0) / 24));
    MTXRotDeg(mrx, 'y',  (DEMOPadGetStickX(0) / 24));
    MTXConcat(mry, ModelMtx, ModelMtx);
    MTXConcat(mrx, ModelMtx, ModelMtx);

    // Light Position Calculation
    LightTheta += (DEMOPadGetSubStickX(0) / 24);
    Clamp(LightTheta, -90, 90);
    LightPhi   += (DEMOPadGetSubStickY(0) / 24);
    Clamp(LightPhi,   -90, 90);

    f_theta = (f32)LightTheta * PI / 180.0F;
    f_phi   = (f32)LightPhi   * PI / 180.0F;
    
    Light1Dir.x = - cosf(f_phi) * sinf(f_theta);
    Light1Dir.y = - sinf(f_phi);
    Light1Dir.z = - cosf(f_phi) * cosf(f_theta);
    Light0Pos.x = - Light1Dir.x * 1000;
    Light0Pos.y = - Light1Dir.y * 1000;
    Light0Pos.z = - Light1Dir.z * 1000;
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
    OSReport("********************************\n");
    OSReport("    Lighting demo on GD\n");
    OSReport("********************************\n");
    OSReport("START      : Quit.\n");
    OSReport("Main Stick : Rotate the model.\n");
    OSReport("Sub  Stick : Move light.\n");
    OSReport("********************************\n");
}

/*============================================================================*/
