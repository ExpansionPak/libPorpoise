/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     lit-basic.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Lighting/lit-basic.c $
    
    13    5/12/01 11:25p Hirose
    removed obsolete flags
    
    12    11/28/00 8:59p Hirose
    Removed Verify/Stat codes
    
    11    8/27/00 5:45p Alligator
    print warning messages through a callback function
    
    10    8/15/00 7:33p Alligator
    changed max verify level to 3
    
    9     7/01/00 2:45p Alligator
    intro stuff
    
    8     6/13/00 11:14a Alligator
    fix warning
    
    7     6/12/00 1:46p Alligator
    updated demo statistics to support new api
    
    6     6/06/00 12:22p Alligator
    use new perf counter api
    
    5     5/24/00 2:52a Alligator
    added button to control verify level to evaluate performance
    
    4     5/22/00 1:56a Alligator
    use GXGetPerfMetric directly for now.
    
    3     5/18/00 3:05a Alligator
    added DEMOStats to this demo for testing
    
    2     3/23/00 4:56p Hirose
    updated to use DEMOPad library
    
    1     3/06/00 12:10p Alligator
    move from tests/gx and rename
    
    7     2/25/00 12:51a Hirose
    updated pad control functions to match actual HW1 gamepad
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     1/28/00 4:06p Hirose
    added singleframe test
    
    4     1/25/00 2:54p Carl
    Changed to standardized end of test message
    
    3     1/18/00 3:39p Hirose
    added GXSetNumChans() and GXSetTevOrder() calls
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    12    11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    11    11/12/99 4:29p Yasu
    Add GXSetNumTexGens(0)  for GX_PASSCLR
    
    10    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    9     9/20/99 3:49p Alligator
    Updated for DEMO lib
    
    8     9/20/99 10:57a Alligator
    remove GXSetShadeMode, update Tlut api
    
    7     9/15/99 12:48p Ryan
    update to fix compiler warnings
    
    6     7/23/99 2:53p Ryan
    changed dolphinDemo.h to demo.h
    
    5     7/23/99 12:16p Ryan
    included dolphinDemo.h
    
    4     7/20/99 6:30p Alligator
    add new models supported by GXDraw
    
    3     7/16/99 1:05p Alligator
    use GXDraw functions
    
    2     7/08/99 4:42p Alligator
    
    1     7/06/99 12:14p Alligator

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

#define PI    3.14159265358979323846

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void CameraInit      ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( Mtx v );
static void AnimTick        ( void );
static void PrintIntro      ( void );
static void SetAmbClr       ( u8 cur );
static void SetMatClr       ( u8 cur );
static void SetModel        ( u8 cur);
static void SetChanEn       ( u8 cur );
static void DumpCurMenu     ( void );

/*---------------------------------------------------------------------------*
  Model Data
 *---------------------------------------------------------------------------*/

typedef struct
{
    char *mode;
    char **opt;
    u8   cursel;
    u8   maxsel; // n opts - 1
    void (*f)(u8 cur);
} MenuOpt;


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

char *ColorOpt[] = {
    "Black",
    "White",
    "Red",
    "Green",
    "Blue",
    "Gray" };

typedef enum {
	CYLINDER,
	TORUS,
	CUBE,
	SPHERE,
	DODECA,
	OCTA,
	ICOSA,
	SPHERE1} MyModels;

char *ModelOpt[] = {
    "Cylinder",
    "Torus",
    "Cube",
    "Sphere",
    "Dodeca",
    "Octa",
    "Icosa",
    "Sphere1" };

char *ChanEnOpt[] = {
    "Disable",
    "Enable" };

MenuOpt Menu[] = {
{ "chan en:    ", ChanEnOpt, 0, 1, SetChanEn },
{ "amb color:  ", ColorOpt,  0, 5, SetAmbClr },
{ "mat color:  ", ColorOpt,  2, 5, SetMatClr },
{ "model:      ", ModelOpt,  0, 7, SetModel  }
};

GXLightObj MyLight;

u8       MyAmbClr = 0;
u8       MyMatClr = 2;
u8       MyChanEn = GX_DISABLE;
MyModels MyModel  = CYLINDER;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    Mtx         v;   // view matrix
   
    DEMOInit(NULL);
    
    PrintIntro();  // Print demo directions
    
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {  
	    DEMOBeforeRender(); // start stats counters

	    CameraInit(v);      // Initialize the camera.  
        DrawInit();         // Define my vertex formats and set array pointers.
        DrawTick(v);        // Draw the model.
        DEMODoneRender();   // Wait until everything is drawn. read stats cntrs.

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
    f32     znear    = 500.0F;
    f32     zfar     = 2000.0F;
    
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
    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, White);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);
}

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
        MyChanEn,    // enable channel
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
    GXSetChanAmbColor(GX_COLOR0, MyColors[MyAmbClr]);
    GXSetChanAmbColor(GX_ALPHA0, White);
    // set up material color
    GXSetChanMatColor(GX_COLOR0A0, MyColors[MyMatClr]);

    // Draw models
    MTXScale(ms, 100.0F, 100.0F, 100.0F);
    MTXConcat(v, ms, mv); 
    MTXRotDeg(mr, axis[axisc % 3], (f32)rot);
    MTXConcat(mv, mr, mv); 
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mvi); 
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    DrawModel(MyModel);

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

    DrawModel(MyModel);

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

    DrawModel(MyModel);

    rot++;
    if (rot == 360)
    {
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
    static s32  curY   = 0;
    static s32  curX   = 0;
    static s32  oldX   = 0; 
    static s32  oldY   = 0; 

    u32  maxY = sizeof(Menu)/sizeof(MenuOpt) - 1;
    u16  down = DEMOPadGetButtonDown(0);
    u8   sel;

    if (down & PAD_BUTTON_Y)
        curY++;
    if (down & PAD_BUTTON_X)
        curY--;
    if (down & PAD_TRIGGER_R)
        curX++;
    if (down & PAD_TRIGGER_L)
        curX--;
    if (down & PAD_BUTTON_A)
        DumpCurMenu();

    // clamp Y (menu)
    if (curY < 0) curY = (s32)maxY;
    if (curY > maxY) curY = 0;

    // clamp X (choice)
    if (curX < 0) curX = Menu[curY].maxsel;
    if (curX > Menu[curY].maxsel) curX = 0;

    sel = Menu[curY].cursel;

    if (curY != oldY || curX != oldX)
    {
        if (curY != oldY)
        {
            curX = sel;
        }
        if (curX != oldX)
        {
            Menu[curY].cursel = (u8)curX;
            Menu[curY].f((u8)curX);
        }
        OSReport("%s %s\n\n\n\n", Menu[curY].mode, Menu[curY].opt[curX]);
    }

    oldX = curX;
    oldY = curY;
}

/*---------------------------------------------------------------------------*
    Name:           DumpCurMenu
    
    Description:    
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DumpCurMenu( void )
{
    u32 i;
    OSReport("Current settings\n");
    for (i = 0; i < sizeof(Menu)/sizeof(MenuOpt); i++)
    {
        OSReport("%s %s\n", Menu[i].mode, Menu[i].opt[Menu[i].cursel]);
    }
    OSReport("\n\n");
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
    OSReport("lit-basic - demonstrate basic lighting controls\n");
    OSReport("***********************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("\n");
    OSReport("Dump current settings BUTTON A\n");
    OSReport("Menu up/down BUTTON Y/X\n");
    OSReport("Menu options TRIGGER L/R\n");
    OSReport("***********************************************\n");
    OSReport("\n\n");

    DumpCurMenu();
}

static void SetAmbClr( u8 cur )
{
    MyAmbClr = cur;
}

static void SetMatClr( u8 cur )
{
    MyMatClr = cur;
}

static void SetModel( u8 cur )
{
    MyModel = (MyModels)cur;
}


static void SetChanEn( u8 cur )
{
    if (cur)
        MyChanEn = GX_ENABLE;
    else
        MyChanEn = GX_DISABLE;
}

/*============================================================================*/
