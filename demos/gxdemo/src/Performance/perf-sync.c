/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     perf-sync.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Performance/perf-sync.c $
    
    2     5/13/01 2:55p Hirose
    removed obsolete flags
    
    1     8/23/00 2:54p Alligator
    demo sampling perf counters in draw sync callback
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>


/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void        main            ( void );
static void CameraInit      ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( Mtx v );
static void AnimTick        ( void );
static void PrintIntro      ( void );

static void myDrawSyncCallback( u16 token );

/*---------------------------------------------------------------------------*
  Model Data
 *---------------------------------------------------------------------------*/

#define Black   MyColors[0]
#define White   MyColors[1]
#define Red     MyColors[2]
#define Green   MyColors[3]
#define Blue    MyColors[4]
#define Gray    MyColors[5]

GXColor MyColors[] = {
    {0x00, 0x00, 0x00, 0x00},  // black
    {0xff, 0xff, 0xff, 0xff},  // white
    {0xff, 0x00, 0x00, 0xff},  // red
    {0x00, 0xff, 0x00, 0xff},  // green
    {0x00, 0x00, 0xff, 0xff},  // blue
    {0x80, 0x80, 0x80, 0xff}}; // gray


typedef enum {
	CYLINDER = 1,
	TORUS,
	CUBE,
	SPHERE,
	DODECA,
	OCTA,
	ICOSA,
	SPHERE1,
    MAX_MODELS 
} MyModels;

char *ModelOpt[] = {
    "Cylinder",
    "Torus",
    "Cube",
    "Sphere",
    "Dodeca",
    "Octa",
    "Icosa",
    "Sphere1" };


GXLightObj myLight;
u32        myCount[MAX_MODELS-1];

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    Mtx         v;   // view matrix
    u32         i;
   
    DEMOInit(NULL);
   
    GXSetDrawSyncCallback( myDrawSyncCallback );
    GXSetGP0Metric(GX_PERF0_CLOCKS);

    for (i = 0; i < MAX_MODELS-1; i++)
        myCount[i] = 0;

    PrintIntro();  // Print demo directions
    
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {  
	    DEMOBeforeRender(); 
        GXClearGP0Metric(); // clear perf counter

	    CameraInit(v);      // Initialize the camera.  
        DrawInit();         // Define my vertex formats and set array pointers.
        DrawTick(v);        // Draw the model.
        DEMODoneRender();   // Wait until everything is drawn.

        DEMOPadRead();      // Update pad status.
        AnimTick();         // Update animation.
    }

    OSHalt("End of demo");
}


/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/
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
    Vec     up      = {0.0F, 1.0F, 0.0F};
    Vec     camLoc  = {0.0F, 0.0F, 800.0F};
    Vec     objPt   = {0.0F, 0.0F, -100.0F};
    f32     left    = 240.0F;
    f32     top     = 320.0F;
    f32     znear   = 500.0F;
    f32     zfar    = 2000.0F;
    
    MTXFrustum(p, left, -left, -top, top, znear, zfar);
    GXSetProjection(p, GX_PERSPECTIVE);
    
    MTXLookAt(v, &camLoc, &up, &objPt);    
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format 0, and sets
                    the array pointers and strides for the indexed data.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{ 
    // for generated models
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    // 
    // set up light parameters
    //

    GXInitLightPos(&myLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&myLight, White);
    GXLoadLightObjImm(&myLight, GX_LIGHT0);
}


/*---------------------------------------------------------------------------*
    Name:           DrawModel
    
    Description:    Draw one of the available models
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel(MyModels model)
{
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

    if (model == CYLINDER)
        GXDrawCylinder(16);
    else if (model == TORUS)
        GXDrawTorus(0.375F, 12, 16);
    else if (model == SPHERE)
        GXDrawSphere(8, 16);
    else if (model == CUBE)
        GXDrawCube();
    else if (model == DODECA)
        GXDrawDodeca();
    else if (model == OCTA)
        GXDrawOctahedron();
    else if (model == ICOSA)
        GXDrawIcosahedron();
    else if (model == SPHERE1)
        GXDrawSphere1(2);
}


/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once. 
                    GXInit makes GX_PNMTX0 the default matrix.
                    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( Mtx v )
{
    static u32  rot = 60;
    static u32  axisc = 0;
    char axis[3] = {'x', 'y', 'z'};

    Mtx  ms;  // Model matrix. scale
    Mtx  mr;  // Model matrix. rotate
    Mtx  mt;  // Model matrix. translate
    Mtx  mv;  // Modelview matrix.
    Mtx  mvi; // Modelview matrix.
  
    // Enable Z compare.  Have to reset because DEMOPrintStats turns off
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);

    // render mode = one color / no texture
    GXSetNumTexGens(0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    //
    // set up channel control
    //
    GXSetNumChans(1); // number of color channels

    GXSetChanCtrl(
        GX_COLOR0,
        GX_TRUE,    // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);

    GXSetChanCtrl(
        GX_ALPHA0,
        FALSE,       // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_NONE,  // diffuse function
        GX_AF_NONE);

    // set up ambient color
    GXSetChanAmbColor(GX_COLOR0, Black);
    GXSetChanAmbColor(GX_ALPHA0, White);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, Green);

    // Draw models
    MTXScale(ms, 100.0F, 100.0F, 100.0F);
    MTXConcat(v, ms, mv); 
    MTXRotDeg(mr, axis[(axisc)%3], (f32)rot);
    MTXConcat(mv, mr, mv); 
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    DrawModel(CYLINDER);
    GXSetDrawSync( CYLINDER );

    MTXTrans(mt, -300.0F, 0.0F, 0.0F);
    MTXConcat(v, mt, mv); 
    MTXRotDeg(mr, axis[(axisc+1)%3], (f32)rot);
    MTXConcat(mv, mr, mv); 
    MTXScale(ms, 100.0F, 100.0F, 100.0F);
    MTXConcat(mv, ms, mv); 
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    DrawModel(CUBE);
    GXSetDrawSync( CUBE );

    MTXTrans(mt, 300.0F, 0.0F, 0.0F);
    MTXConcat(v, mt, mv); 
    MTXRotDeg(mr, axis[(axisc+2)%3], (f32)rot);
    MTXConcat(mv, mr, mv); 
    MTXScale(ms, 100.0F, 100.0F, 100.0F);
    MTXConcat(mv, ms, mv); 
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    DrawModel(TORUS);
    GXSetDrawSync( TORUS );

    rot++;
    if (rot == 360) {
        rot = 0;
        axisc++;
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Menu of test options
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( void )
{
    u8   i;
    u16  down = DEMOPadGetButtonDown(0);

    if (down & PAD_BUTTON_A)
    {
        for (i = 0; i < MAX_MODELS-1; i++)
            OSReport("%s done at %d clocks\n", ModelOpt[i], myCount[i]);
        OSReport("\n");
    }
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
    OSReport("***********************************************\n");
    OSReport("perf-sync - demonstrates sampling perf counters \n");
    OSReport("            using the draw sync callback.       \n");
    OSReport("***********************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("BUTTON A, dump current stats\n");
    OSReport("***********************************************\n");
    OSReport("\n\n");
}


/*---------------------------------------------------------------------------*
    Name:           myDrawSyncCallback
    
    Description:    This function is called for each drawsync interrupt
                    It stores the current value of the GP0 counter in an
                    array at the location specified by the token value.

    Arguments:      token, the value sent by GXSetDrawSync
 *---------------------------------------------------------------------------*/
void myDrawSyncCallback( u16 token )
{
    if (token >= MAX_MODELS) return;

    myCount[token-1] = GXReadGP0Metric();
}

/*============================================================================*/
