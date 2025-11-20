/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     DL-geom.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/DisplayList/DL-geom.c $
    
    6     2/15/01 11:30a Hirose
    removed obsolete ifdefs
    
    5     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    4     3/23/00 1:42a Hirose
    updated to use DEMOPad library
    changed "start button" to "menu button"
    
    3     3/21/00 3:24p Hirose
    deleted PADInit() call because this function is called once in
    DEMOInit()
    
    2     3/06/00 4:33p Hirose
    fixed demo names referenced in the source code
    
    1     3/06/00 12:08p Alligator
    move from gx/tests and rename
    
    12    3/03/00 4:21p Alligator
    integrate with ArtX source
    
    11    3/02/00 5:35p Alligator
    ifdef for BUG_TRIANGLE_FAN
    
    10    2/24/00 7:06p Yasu
    Rename gamepad key to match HW1
    
    9     2/07/00 5:29p Carl
    Added more display list objects for EPPC version.
    
    8     2/03/00 6:21p Carl
    Fixed tevorder settings.
    
    7     2/03/00 5:48p Carl
    Fixed cache-coherency issue.
    
    6     1/26/00 5:06p Carl
    Fixed white space problem.
    
    5     1/25/00 2:46p Carl
    Changed to standardized end of test message
    
    4     1/18/00 2:37p Hirose
    added GXSetNumChans(1)
    
    3     1/13/00 8:53p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    2     12/02/99 4:44p Carl
    Fixed cache-coherence issue.
    
    1     11/30/99 12:02p Carl
    Display list test with actual geometry.
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>
#include <string.h> // needed for memcpy()

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( void );
static void DrawInit        ( void );
static void PrintIntro      ( void );
static void DrawTick        ( void );
static void VertexLightInit ( void );

static void AnimTick        ( void );

static void SendVertex ( u8 posIndex, u8 normalIndex,
                         u8 colorIndex, u8 texCoordIndex );

void MyCreateSphere(u8 numMajor, u8 numMinor);
void MyDrawSphere(u8 numMajor, u8 numMinor);
void MyDrawCube(void);

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/
#define PI           3.14159265358979323846F
#define SIDE         50
#define WOOD1_TEX_ID 5
#define TMP_SIZE     65536
#define SPHERE_PTS   100
#define NUM_LISTDL   14

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
Mtx v;

TEXPalettePtr tpl = 0;

u32 rot = 0;

// display list stuff goes here.
u8* cubeDL;
u8* sphereDL;
u8* listDL[NUM_LISTDL];
u32 cubeSize;
u32 sphereSize;
u32 listSize[NUM_LISTDL];

/*---*/

GXVtxDescList    vcd[3][GX_MAX_VTXDESCLIST_SZ];

float SFloatVert[SPHERE_PTS][3] ATTRIBUTE_ALIGN(32);

float CFloatVert[] ATTRIBUTE_ALIGN(32) =  
                     {  -SIDE, SIDE, -SIDE,
                        -SIDE, SIDE, SIDE,
                        -SIDE, -SIDE, SIDE,
                        -SIDE, -SIDE, -SIDE,
                        SIDE, SIDE, -SIDE,
                        SIDE, -SIDE, -SIDE,
                        SIDE, -SIDE, SIDE,
                        SIDE, SIDE, SIDE
                    };

float CFloatNorm[] ATTRIBUTE_ALIGN(32) = 
                    {   -1.0F, 0.0F, 0.0F,
                        1.0F, 0.0F, 0.0F,
                        0.0F, -1.0F, 0.0F,
                        0.0F, 1.0F, 0.0F,
                        0.0F, 0.0F, -1.0F,
                        0.0F, 0.0F, 1.0F                        
                    };

u8  CColorRGBA8[] ATTRIBUTE_ALIGN(32) =  
                        {   255, 0, 0, 255, 
                            255, 0, 0, 192, 
                            255, 0, 0, 128, 
                            255, 0, 0, 64, 
                            255, 0, 0, 0};      //GX_RGBA8

float CFloatTex[] ATTRIBUTE_ALIGN(32) =  
                    {   0.0F, 0.0F, 
                        1.0F, 0.0F,
                        1.0F, 1.0F, 
                        0.0F, 1.0F
                    };

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();         // Define my vertex formats and set array pointers.

    PrintIntro();  // Print demo directions

    VertexLightInit();

    DEMOPadRead();     // Read the joystick for this frame

    // While the quit button is not pressed
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {
        DEMOPadRead();  // Read the joystick for this frame

        AnimTick();     // Do animation based on input
        
        DEMOBeforeRender();

        DrawTick();     // Draw the model.

        DEMODoneRender();
    }

    OSHalt("End of demo");
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
    
    MTXFrustum(p, 240, -240, -320, 320, 500, 2000);

    GXSetProjection(p, GX_PERSPECTIVE);

    MTXLookAt(v, &camPt, &up, &origin); 
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Calls the correct initialization function for the current 
                    model.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{
    u8* tmpDL;
    GXTexObj to;
#ifndef EMU
    u16 i;
#endif

    CameraInit();   // Initialize the camera.

    GXClearVtxDesc();

    // set position params
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetArray(GX_VA_POS, CFloatVert, 12);

    // set color params
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetArray(GX_VA_CLR0, CColorRGBA8, 4);

    // set normal params
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetArray(GX_VA_NRM, CFloatNorm, 12);

    // set tex coord params
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    GXSetArray(GX_VA_TEX0, CFloatTex, 8);

    // save the current VCD
    GXGetVtxDescv(vcd[0]);

    // make cube display list
    // note that the display-list buffer area must be forced out of
    // the CPU cache since it will be written using the write-gather pipe
    tmpDL = OSAlloc(TMP_SIZE);
    ASSERTMSG(tmpDL != NULL, "error allocating tmpDL");
    DCInvalidateRange( (void *) tmpDL, TMP_SIZE);

    GXBeginDisplayList( (void *) tmpDL, (u32) TMP_SIZE);
    MyDrawCube();
    cubeSize = GXEndDisplayList();
    cubeDL = OSAlloc(cubeSize);
    ASSERTMSG(cubeDL != NULL, "error allocating cubeDL");
    memcpy( (void *) cubeDL, (void *) tmpDL, cubeSize);
    DCFlushRange( (void *) cubeDL, cubeSize);
    
    // set up VCD/VAT for sphere drawing
    GXClearVtxDesc();

    // set position params
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetArray(GX_VA_POS, SFloatVert, 12);

    // set normal params
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GXSetArray(GX_VA_NRM, SFloatVert, 12);

    // save the current VCD
    GXGetVtxDescv(vcd[1]);

    // generate sphere coordinates
    MyCreateSphere(8, 12);
    DCFlushRange( (void *) SFloatVert, sizeof(SFloatVert) );

    // make sphere display list
    // Note: must get tmpDL out of the cpu cache again
    // (it got back into the cache because of the memcpy)
    DCInvalidateRange( (void *) tmpDL, TMP_SIZE);
    GXBeginDisplayList( (void *) tmpDL, (u32) TMP_SIZE);
    MyDrawSphere(8, 12);
    sphereSize = GXEndDisplayList();
    sphereDL = OSAlloc(sphereSize);
    ASSERTMSG(sphereDL != NULL, "error allocating sphereDL");
    memcpy( (void *) sphereDL, (void *) tmpDL, sphereSize);
    DCFlushRange( (void *) sphereDL, sphereSize);
    
#ifndef EMU
    // set up VCD/VAT for misc. object drawing
    GXClearVtxDesc();

    // set position params
    GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);

    // set normal params
    GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);

    // save the current VCD
    GXGetVtxDescv(vcd[2]);

    // make misc. object display lists

    for(i=0; i<NUM_LISTDL; i++) 
    {
        DCInvalidateRange( (void *) tmpDL, TMP_SIZE);
        GXBeginDisplayList( (void *) tmpDL, (u32) TMP_SIZE);
        switch(i % 7) 
        {           
          case 0: GXDrawCylinder(20); break;
          case 1: GXDrawTorus((f32) 0.5, 12, 16);  break;
          case 2: GXDrawSphere(12, 16); break;
          case 3: GXDrawCube(); break;
          case 4: GXDrawDodeca(); break;
          case 5: GXDrawOctahedron(); break;
          case 6: GXDrawIcosahedron(); break;
        }
        listSize[i] = GXEndDisplayList();
        listDL[i] = OSAlloc(listSize[i]);
        ASSERTMSG(listDL[i] != NULL, "error allocating listDL");
        memcpy( (void *) listDL[i], (void *) tmpDL, listSize[i]);
        DCFlushRange( (void *) listDL[i], listSize[i]);
    }
#endif

    // technically, tmpDL should be freed at this point...

    // Pixel processing setup
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_FALSE,   // enable channel
        GX_SRC_REG, // amb source
        GX_SRC_VTX, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);

    // cube texture setup
    GXSetNumTevStages(1);
    TEXGetPalette(&tpl, "gxTextrs.tpl");
    TEXGetGXTexObjFromPalette(tpl, &to, WOOD1_TEX_ID);
    GXLoadTexObj(&to, GX_TEXMAP0);

    // set blend op
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, 
                   GX_LO_CLEAR /* ignored for blend */);
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
    OSReport("DL-geom: display list test with geometry\n");
    OSReport("************************************************\n");
    OSReport("The top two objects are drawn using display lists.\n");
    OSReport("The bottom two objects are drawn normally.\n");
    OSReport("\n");
    OSReport("To quit hit the start button.\n");
    OSReport("\n");
    OSReport("A Button     : Hold to pause the animation\n");
    OSReport("************************************************\n");
    OSReport("\n");
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
#ifndef EMU
    u16 i;
    GXColor other  = {  0,   0,   0, 255};
#endif
    Mtx mv, r1, r2, mr, ms, mt;
    GXColor yellow = {255, 255,   0, 255};
    GXColor cyan   = {  0, 255, 255, 255};

    // cube setup

    GXSetVtxDescv(vcd[0]);
    GXSetArray(GX_VA_POS, CFloatVert, 12);
    GXSetArray(GX_VA_NRM, CFloatNorm, 12);

    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0,
        GX_TRUE,    // enable channel
        GX_SRC_REG, // amb source
        GX_SRC_VTX, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);

    // Draw regular cube

    MTXTrans(mt, -250, -200, 0);
    MTXRotDeg(r1, 'X', (float)rot);
    MTXRotDeg(r2, 'y', (float)rot/2.0F);
    MTXConcat(r2, r1, mr);
    MTXConcat(mt, mr, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    
    MyDrawCube();
    
    // Draw DL cube

    MTXTrans(mt, -250, 200, 0);
    MTXConcat(mt, mr, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    
    GXCallDisplayList(cubeDL, cubeSize);
    
    // sphere setup

    GXSetVtxDescv(vcd[1]);
    GXSetArray(GX_VA_POS, SFloatVert, 12);
    GXSetArray(GX_VA_NRM, SFloatVert, 12);

    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0,
        GX_TRUE,    // enable channel
        GX_SRC_REG, // amb source
        GX_SRC_REG, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    // Draw regular sphere

    GXSetChanMatColor(GX_COLOR0A0, yellow);

    MTXTrans(mt, 250, -200, 0);
    MTXScale(ms, 60, 60, 60);
    MTXConcat(mr, ms, mv);
    MTXConcat(mt, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    
    MyDrawSphere(8, 12);

    // Draw DL sphere

    GXSetChanMatColor(GX_COLOR0A0, cyan);

    MTXTrans(mt, 250, 200, 0);
    MTXConcat(mr, ms, mv);
    MTXConcat(mt, mv, mv);
    MTXConcat(v, mv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    
    GXCallDisplayList(sphereDL, sphereSize);

#ifndef EMU
    // Draw list objects

    GXSetVtxDescv(vcd[2]);

    for(i=0; i<NUM_LISTDL; i++) 
    {
        other.r = (u8) (i*255/NUM_LISTDL);
        other.g = (u8) (i*127/NUM_LISTDL);
        other.b = (u8) (255-other.g);
        
        GXSetChanMatColor(GX_COLOR0A0, other);
        
        MTXTrans(mt, (f32) (-275.0+550.0*i/(NUM_LISTDL-1)), 80*(i&1)-40, 0);
        MTXScale(ms, 30, 30, 30);
        MTXConcat(mr, ms, mv);
        MTXConcat(mt, mv, mv);
        MTXConcat(v, mv, mv);
        GXLoadPosMtxImm(mv, GX_PNMTX0);
    
        GXCallDisplayList(listDL[i], listSize[i]);
    }
#endif
}

/*---------------------------------------------------------------------------*/
static void VertexLightInit ( void )
{
    GXLightObj MyLight;
    GXColor litcolor = {255, 255, 255, 255}; // white
    
    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, litcolor);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);
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
    u16 buttons = DEMOPadGetButton(0);
    s8 stickX = DEMOPadGetStickX(0);
    s8 stickY = DEMOPadGetStickY(0);

    if(!(buttons & PAD_BUTTON_A))
    {
        rot ++;
        if(rot > 719) 
        {
            rot = 0;
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:           MyCreateSphere
    
    Description:    Creates a matrix containing sphere coordinates.
                    
    Arguments:      numMajor - number of major steps
                    numMinor - number of minor steps

    Returns:        none
 *---------------------------------------------------------------------------*/
void MyCreateSphere(u8 numMajor, u8 numMinor)
{
  f32 majorStep = (PI / numMajor);
  f32 minorStep = (2.0F * PI / numMinor);
  s32 i, j, v=0;

  // top point
  SFloatVert[v][0] = (f32) 0.0;
  SFloatVert[v][1] = (f32) 0.0;
  SFloatVert[v][2] = (f32) 1.0;
  v++;
  for (i = 1; i < numMajor; i++) 
  {
    f32 a = i * majorStep;
    f32 r0 = sinf(a);
    f32 z0 = cosf(a);

    for (j = 0; j < numMinor; j++)
    {
      f32 c = j * minorStep;
      f32 x = cosf(c);
      f32 y = sinf(c);

      SFloatVert[v][0] = x * r0;
      SFloatVert[v][1] = y * r0;
      SFloatVert[v][2] = z0;
      v++;
    }
  }

  SFloatVert[v][0] = (f32)  0.0;
  SFloatVert[v][1] = (f32)  0.0;
  SFloatVert[v][2] = (f32) -1.0;
}

/*---------------------------------------------------------------------------*
    Name:           MyDrawSphere
    
    Description:    Draw a sphere.  Sends indices to sphere matrix.
                    
    Arguments:      numMajor - number of major steps
                    numMinor - number of minor steps

    Returns:        none
 *---------------------------------------------------------------------------*/
void MyDrawSphere(u8 numMajor, u8 numMinor)
{
  s32 i, j, v=0;
  s32 n;

#ifdef BUG_TRIANGLE_FAN
  // top hat
  for (j = numMinor; j > 0; j--)
  {
  GXBegin(GX_TRIANGLES, GX_VTXFMT1, 3);
      GXPosition1x16((u16) 0);
      GXNormal1x16((u16) 0);

      GXPosition1x16((u16) (j % numMinor + 1));
      GXNormal1x16((u16) (j % numMinor + 1));

      GXPosition1x16((u16) (j % (numMinor + 1)));
      GXNormal1x16((u16) (j % (numMinor + 1)));
  GXEnd();
  }
#else
  // top hat
  GXBegin(GX_TRIANGLEFAN, GX_VTXFMT1, (u16) (numMinor+2));
  GXPosition1x16((u16) 0);
  GXNormal1x16((u16) 0);
  for (j = numMinor; j >= 0; j--)
  {
      GXPosition1x16((u16) (j % numMinor + 1));
      GXNormal1x16((u16) (j % numMinor + 1));
  }
  GXEnd();
#endif // BUG_TRIANGLE_FAN

  // central portion
  for (i = 1; i < numMajor-1; i++) 
  {
    n = (i-1) * numMinor + 1;
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, (u16) ((numMinor+1)*2));
    for (j = 0; j <= numMinor; j++)
    {
      GXPosition1x16((u16) (n + numMinor + j % numMinor));
      GXNormal1x16((u16) (n + numMinor + j % numMinor));
      GXPosition1x16((u16) (n + j % numMinor));
      GXNormal1x16((u16) (n + j % numMinor));
    }
    GXEnd();
  }

  // bottom cup
  n = (numMajor-1) * numMinor;

#ifdef BUG_TRIANGLE_FAN
  for (j = numMinor; j >= 0; j--)
  {
  GXBegin(GX_TRIANGLES, GX_VTXFMT1, 3);
      GXPosition1x16((u16) (n+1));
      GXNormal1x16((u16) (n+1));

      GXPosition1x16((u16) (n - ((j+1) % numMinor)));
      GXNormal1x16((u16) (n - ((j+1) % numMinor)));

      GXPosition1x16((u16) (n - j % numMinor));
      GXNormal1x16((u16) (n - j % numMinor));
  GXEnd();
  }
#else
  GXBegin(GX_TRIANGLEFAN, GX_VTXFMT1, (u16) (numMinor+2));
  GXPosition1x16((u16) (n+1));
  GXNormal1x16((u16) (n+1));
  for (j = numMinor; j >= 0; j--)
  {
      GXPosition1x16((u16) (n - j % numMinor));
      GXNormal1x16((u16) (n - j % numMinor));
  }
  GXEnd();
#endif // BUG_TRIANGLE_FAN
}

/*---------------------------------------------------------------------------*/

static void SendVertex ( u8 posIndex, u8 normalIndex,
                         u8 colorIndex, u8 texCoordIndex )
{
    GXPosition1x8(posIndex);
    GXNormal1x8(normalIndex);
    GXColor1x8(colorIndex);
    GXTexCoord1x8(texCoordIndex);
}

/*---------------------------------------------------------------------------*
    Name:           MyDrawCube
    
    Description:    Draws a cube.  Position, normal, etc. are indirect
                    
    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
void MyDrawCube(void)
{
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 6*6);

        SendVertex(0, 0, 1, 0);
        SendVertex(1, 0, 2, 1);
        SendVertex(2, 0, 3, 2);
        SendVertex(2, 0, 3, 2);
        SendVertex(3, 0, 4, 3);
        SendVertex(0, 0, 1, 0);

        SendVertex(4, 1, 0, 0);
        SendVertex(5, 1, 0, 1);
        SendVertex(6, 1, 0, 2);
        SendVertex(6, 1, 0, 2);
        SendVertex(7, 1, 0, 3);
        SendVertex(4, 1, 0, 0);

        SendVertex(2, 2, 0, 0);
        SendVertex(6, 2, 0, 1);
        SendVertex(5, 2, 0, 2);
        SendVertex(5, 2, 0, 2);
        SendVertex(3, 2, 0, 3);
        SendVertex(2, 2, 0, 0);
        
        SendVertex(1, 3, 0, 0);
        SendVertex(0, 3, 0, 1);
        SendVertex(4, 3, 0, 2);
        SendVertex(4, 3, 0, 2);
        SendVertex(7, 3, 0, 3);
        SendVertex(1, 3, 0, 0);       
        
        SendVertex(5, 4, 0, 0);
        SendVertex(4, 4, 0, 1);
        SendVertex(0, 4, 0, 2);
        SendVertex(0, 4, 0, 2);
        SendVertex(3, 4, 0, 3);
        SendVertex(5, 4, 0, 0);

        SendVertex(6, 5, 0, 0);
        SendVertex(2, 5, 0, 1);
        SendVertex(1, 5, 0, 2);
        SendVertex(1, 5, 0, 2);
        SendVertex(7, 5, 0, 3);
        SendVertex(6, 5, 0, 0);

    GXEnd();
}

/****************************************************************************/
/****************************************************************************/
