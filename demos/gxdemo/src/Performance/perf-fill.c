/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     fill-perf.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  Fill-rate performance test

  $Log: /Dolphin/build/demos/gxdemo/src/Performance/perf-fill.c $
    
    5     7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    4     11/28/00 9:40p Hirose
    Update due to DEMOStat library enhancements
    
    3     8/23/00 2:53p Alligator
    allow perf counter api to work with draw sync callback
    
    2     6/26/00 6:39p Alligator
    add GXEnd to draw arrow code
    
    1     6/20/00 11:55a Alligator
    initial performance demos
    
    3     6/20/00 10:37a Alligator
    added texture bandwidth, texture miss rate calculations
    
    2     6/19/00 1:44p Alligator
    added control for texture caching method, number of textures/cube, more
    stats, etc.
    
    1     6/13/00 1:59p Alligator
    test for fill rate, initial version
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main             ( void );
static void CameraInit       ( void );
static void ViewInit         ( void );
static void DrawInit         ( void );
static void InitDrawState    ( void );
static void DrawTick         ( void );
static void AnimTick         ( void );
static void MakeModelMtx     ( Vec xAxis, Vec yAxis, Vec zAxis, Mtx m );
static void PrintIntro       ( void );
static void DrawCubeIndx     ( void );
static void DrawCubeDirect   ( void );
static void DrawArrow        ( void );
static void init_rand        ( void );
static u32  lrand            ( void );
static f32  frand            ( void );

static GXTexRegion* 
            MyTexRegionCallback( GXTexObj* texObj, GXTexMapID mapID );

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/
#define INDEXED_DATA    1 // comment out for direct data

#define TCACHE_SZ       GX_TEXCACHE_32K
#define NUM_REGIONS     8 // number of caches
#define NUM_TEXOBJS     9 // for 8 tev stages, plus 1 to swap between
#define MAX_TEV         8 // for this test
#define NUM_ROTMATS    10 // just because

#define STAT_SIZE       (sizeof(MyStats) / sizeof(DEMOStatObj))
#define SIDE            50.0F


/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
//
//  Cube data
// 
f32 Vert[] ATTRIBUTE_ALIGN(32) =  
{  
    -SIDE,  SIDE, -SIDE, //0
    -SIDE,  SIDE,  SIDE, //1
    -SIDE, -SIDE,  SIDE, //2
    -SIDE, -SIDE, -SIDE, //3
     SIDE,  SIDE, -SIDE, //4
     SIDE, -SIDE, -SIDE, //5
     SIDE, -SIDE,  SIDE, //6
     SIDE,  SIDE,  SIDE, //7
        0,     0, -SIDE, //8
     SIDE/4,   0,     0, //9
    -SIDE/4,   0,     0, //10
        0, SIDE/4,    0  //11
};

f32 Norm[] ATTRIBUTE_ALIGN(32) =  
{  
    -1.0F,  0.0F,  0.0F, //0
     0.0F, -1.0F,  0.0F, //1
     0.0F,  0.0F, -1.0F, //2
     1.0F,  0.0F,  0.0F, //3
     0.0F,  1.0F,  0.0F, //4
     0.0F,  0.0F,  1.0F  //5
};

f32 TexCoord[] ATTRIBUTE_ALIGN(32) =  
{  
    0.0F,  0.0F, //0
    1.0F,  0.0F, //1
    1.0F,  1.0F, //2
    0.0F,  1.0F, //3
};

u32 MyColors[] ATTRIBUTE_ALIGN(32) = 
{
    0xff0000ff, // red
    0x00ff00ff, // green
    0x0000ffff, // blue
    0xffff00ff, // yellow
    0xff00ffff, // magenta
    0x00ffffff  // cyan
};

//
//  Test structure
//
typedef struct
{
    u32 numx;       // number of cubes in x direction
    u32 numy;       // number of cubes in y direction
    u32 numz;       // number of cubes in z direction
    f32 dx;         // dist between cubes in x direction
    f32 dy;         // dist between cubes in y direction
    f32 dz;         // dist between cubes in z direction
    u8  ntev;       // number of textures per cube (num tev stages)
    u32 nc_tx;      // number of cubes per texture change
    u32 tcb;        // texture region allocator callback
    u32 tci;        // invalidate tex region on each GXLoadTexObj when tcb=1
    f32 mods;       // model scale
    f32 rot;        // model rotation
    f32 dr;         // model rotation speed
    Vec axis[NUM_ROTMATS];   // random rotation axis, initialized by DrawInit
} TestData;

static TestData testData;

//
//  Statistics
//
//
DEMOStatObj MyStats1[] =
{
    // General Performance
    {  "GP Clocks [1/60]..",  DEMO_STAT_GP0, GX_PERF0_CLOCKS,    0 },
    {  "Vertices..........",  DEMO_STAT_GP0, GX_PERF0_VERTICES,  0 },
    {  "Fill Rate [MP/Sec]",  DEMO_STAT_FR,  GX_PERF0_NONE,      0 },
    {  "TextureBW [MB/Sec]",  DEMO_STAT_TBW, GX_PERF0_NONE,      0 },
    {  "TexB/pix..........",  DEMO_STAT_TBP, GX_PERF0_NONE,      0 },
};

DEMOStatObj MyStats2[] =
{
    // Texture Performance Detail
    {  "Texels.........",  DEMO_STAT_GP1, GX_PERF1_TEXELS,       0 },
    {  "Tex 1-2 Lines..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK1_2,  0 },
    {  "Tex 3-4 Lines..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK3_4,  0 },
    {  "Tex 5-6 Lines..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK5_6,  0 },
    {  "Tex 7-8 Lines..",  DEMO_STAT_GP1, GX_PERF1_TC_CHECK7_8,  0 },
    {  "Tex Cache Miss.",  DEMO_STAT_GP1, GX_PERF1_TC_MISS,      0 },   
    {  "Total Checks...",  DEMO_STAT_MYC, GX_PERF1_NONE,         0 },
    {  "Tex$ Miss Rate.",  DEMO_STAT_MYR, GX_PERF1_NONE,         0 },
};

DEMOStatObj MyStats3[] =
{
    // Pixel Performance Detail
    {  "Pixels Entered.",  DEMO_STAT_PIX, DEMO_STAT_PIX_TI,      0 },
    {  "Pixels Z Passed",  DEMO_STAT_PIX, DEMO_STAT_PIX_TO,      0 },
    {  "Pixels Blended.",  DEMO_STAT_PIX, DEMO_STAT_PIX_CI,      0 },
    {  "Quads w>0 Cvg..",  DEMO_STAT_GP0, GX_PERF0_QUAD_NON0CVG, 0 },
    {  "Quads w/1 Cvg..",  DEMO_STAT_GP0, GX_PERF0_QUAD_1CVG,    0 },
    {  "Quads w/2 Cvg..",  DEMO_STAT_GP0, GX_PERF0_QUAD_2CVG,    0 },
    {  "Quads w/3 Cvg..",  DEMO_STAT_GP0, GX_PERF0_QUAD_3CVG,    0 },
    {  "Quads w/4 Cvg..",  DEMO_STAT_GP0, GX_PERF0_QUAD_4CVG,    0 },
};

DEMOStatObj MyStats4[] =
{
    // Memory Access Detail
    {  "CP Reqs........",  DEMO_STAT_MEM, DEMO_STAT_MEM_CP,      0 },
    {  "TX Cache Reqs..",  DEMO_STAT_MEM, DEMO_STAT_MEM_TC,      0 },
    {  "CPU Rd Reqs....",  DEMO_STAT_MEM, DEMO_STAT_MEM_CPUR,    0 },
    {  "CPU Wr Reqs....",  DEMO_STAT_MEM, DEMO_STAT_MEM_CPUW,    0 },
    {  "DSP Reqs.......",  DEMO_STAT_MEM, DEMO_STAT_MEM_DSP,     0 },
    {  "IO Reqs........",  DEMO_STAT_MEM, DEMO_STAT_MEM_IO,      0 },
    {  "VI Reqs........",  DEMO_STAT_MEM, DEMO_STAT_MEM_VI,      0 },
    {  "PE Reqs........",  DEMO_STAT_MEM, DEMO_STAT_MEM_PE,      0 },
    {  "Refresh Reqs...",  DEMO_STAT_MEM, DEMO_STAT_MEM_RF,      0 },
    {  "Forced Idle....",  DEMO_STAT_MEM, DEMO_STAT_MEM_FI,      0 },
};

DEMOStatObj MyStats5[] =
{
    // Current Setting
    {  "Num Of Cubes...",  DEMO_STAT_MYC, 0,      0 },
    {  "Cubes/Texture..",  DEMO_STAT_MYC, 0,      0 },
    {  "Num TevStages..",  DEMO_STAT_MYC, 0,      0 },
};

#define STAT_MENU0  0
#define STAT_MENU1  (sizeof(MyStats1)/sizeof(DEMOStatObj))
#define STAT_MENU2  (sizeof(MyStats2)/sizeof(DEMOStatObj))
#define STAT_MENU3  (sizeof(MyStats3)/sizeof(DEMOStatObj))
#define STAT_MENU4  (sizeof(MyStats4)/sizeof(DEMOStatObj))
#define STAT_MENU5  (sizeof(MyStats5)/sizeof(DEMOStatObj))

//
//  Global vars
//
static Mtx v, mv;

static u32 animMode  = 0;
static u32 showStats = 1; // show fill rate by default
static u32 stopAnim  = 0;
static void       (*testDraw)() = NULL; // function pointer for draw routine
static GXTexObj     texObj[NUM_TEXOBJS];  // texture object
static GXTexRegion  MyTexRegions[NUM_REGIONS]; // cache regions

static GXTexRegionCallback oldCallback;

static GXLightObj   MyLight; // light object

static Vec CamX = {1.0F, 0.0F, 0.0F};	
static Vec CamY = {0.0F, 1.0F, 0.0F};
static Vec CamZ = {0.0F, 0.0F, 1.0F};
static f32 CamS = 100.0F; // camera zoom factor

static OSStopwatch myTotTime;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
    DrawInit(); 
    PrintIntro();

    OSInitStopwatch(&myTotTime, "myTotTime");
    OSResetStopwatch(&myTotTime);

    testDraw = DrawCubeIndx;

    while (!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {
        DEMOPadRead();
        AnimTick();    

        InitDrawState(); 
        CameraInit();

        OSStartStopwatch(&myTotTime);
        DEMOBeforeRender();

        DrawTick();
        
        if ( showStats == 2 )
        {
            // total check count
            MyStats2[6].count = MyStats2[1].count + MyStats2[2].count + 
                                MyStats2[3].count + MyStats2[4].count;
            // compute miss rate
            MyStats2[7].stat  = MyStats2[5].count; // misses
            MyStats2[7].count = MyStats2[6].count; // total checks
        }
        else if ( showStats == 5 )
        {
            MyStats5[0].count = testData.numx * testData.numy * testData.numz;
            MyStats5[1].count = testData.nc_tx;
            MyStats5[2].count = testData.ntev;
        }

        GXDrawDone();
        OSStopStopwatch(&myTotTime);

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
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit ( void )
{
    Mtx44 p;
    
    MTXFrustum(p, .24F * CamS,-.24F * CamS,
               -.32F * CamS, .32F * CamS, 
               .5F * CamS, 40.0F * CamS);
    
    GXSetProjection(p, GX_PERSPECTIVE);
    ViewInit();
}


/*---------------------------------------------------------------------------*
    Name:           ViewInit
    
    Description:    Initialize the view matrix.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void ViewInit ( void )
{
    Mtx trans;

    MakeModelMtx(CamX, CamY, CamZ, v);	// Make a new view matrix
    MTXTranspose(v, v);
    MTXTrans(trans, 0.0F, 0.0F, -8.0F * CamS);
    MTXConcat(trans, v, v);
}


/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initialize one-time state settings
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{
    u32           i;
    GXColor       color = {0, 255, 255, 255};
    TEXPalettePtr tpl = 0; // texture palette 

    //
    //  Initialize test structure
    //
    testData.numx  = 1;
    testData.numy  = 1;
    testData.numz  = 1;
    testData.nc_tx = testData.numx * testData.numz;
    testData.tcb   = 0; // default texregion callback
    testData.tci   = 0; // don't invlidate tex cache regions
    testData.dx    = 2*SIDE + 10.0F;
    testData.dy    = 2*SIDE + 10.0F;
    testData.dz    = -(2*SIDE + 10.0F);
    testData.ntev  = 1;      // num textures
    testData.mods  = 1.0F;   // model scale
    testData.rot   = 0.0F;   // model rotation
    testData.dr    = 1.0F;   // rotation speed

    DEMOSetStats(MyStats1, STAT_MENU1, DEMO_STAT_TLD);

    // init texture regions
    for ( i = 0 ; i < NUM_REGIONS ; i++ )
    {
        // The region is used as a 32K cache.
        GXInitTexCacheRegion(
            &MyTexRegions[i],
            GX_FALSE,               // 32b mipmap
            0x00000 + i * 0x08000,  // tmem_even
            TCACHE_SZ,              // size_even
            0x80000 + i * 0x08000,  // tmem_odd
            TCACHE_SZ );            // size_odd
    }

    // initialize oldCallback to default callback
    oldCallback = GXSetTexRegionCallback(MyTexRegionCallback);
    GXSetTexRegionCallback(oldCallback);

    // model axis
    init_rand();
    for (i = 0; i < NUM_ROTMATS; i++)
    {   
        // make random axis
        testData.axis[i].x = frand();
        testData.axis[i].y = frand();
        testData.axis[i].z = frand();
        VECNormalize(&testData.axis[i], &testData.axis[i]);
    }

    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, color);

    //  Load the texture palette
    TEXGetPalette(&tpl, "gxTests/tex-07.tpl");

    //  Initialize a texture object to contain the correct texture
    for (i = 0; i < NUM_TEXOBJS; i++)
    {
        OSReport("Reading tex obj %d\n", i);
        TEXGetGXTexObjFromPalette(tpl, &texObj[i], i);
        GXInitTexObjUserData(&texObj[i], (void*)i);
    }

    InitDrawState();

    GXDrawDone(); // make sure state is in GP
}


/*---------------------------------------------------------------------------*
    Name:           InitDrawState
    
    Description:    Sets drawing state that gets destroyed by DEMOPrintf
                    when the statistics are printed.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void InitDrawState( void )
{
    GXColor       color = {0, 255, 0, 255};

    GXSetCurrentMtx(0);
    GXSetCullMode(GX_CULL_BACK);

    GXLoadLightObjImm(&MyLight, GX_LIGHT0);

    GXSetChanMatColor(GX_COLOR0A0, color);
    GXSetChanCtrl(
                GX_COLOR0A0,
                GX_FALSE,       // enable channel
                GX_SRC_REG,     // amb source
                GX_SRC_VTX,     // mat source
                GX_LIGHT_NULL,  // light mask
                GX_DF_NONE,     // diffuse function
                GX_AF_NONE);

    //  Load the texture object; tex0 is used in stage 0
#if 0
    for (i = 0; i < 8; i++)
        GXLoadTexObj(&texObj[i], (GXTexMapID)i);
#endif

    GXSetNumChans(1);
    GXSetNumTexGens(1);
    GXSetNumTevStages(testData.ntev);

    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD0, GX_TEXMAP3, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD0, GX_TEXMAP4, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE5, GX_TEXCOORD0, GX_TEXMAP5, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE6, GX_TEXCOORD0, GX_TEXMAP6, GX_COLOR_NULL);
    GXSetTevOrder(GX_TEVSTAGE7, GX_TEXCOORD0, GX_TEXMAP7, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE1, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE2, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE3, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE4, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE5, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE6, GX_REPLACE);
    GXSetTevOp(GX_TEVSTAGE7, GX_REPLACE);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_ENABLE);

    GXClearVtxDesc();

#ifdef INDEXED_DATA
    //OSReport("Using indexed data\n");
    GXSetVtxDesc(GX_VA_POS,  GX_INDEX16);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
#else
    //OSReport("Using direct data\n");
    GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
#endif // INDEXED_DATA

    GXSetArray(GX_VA_POS,  Vert,     sizeof(f32)*3);
    GXSetArray(GX_VA_CLR0, MyColors, sizeof(u32));
    GXSetArray(GX_VA_TEX0, TexCoord, sizeof(f32)*2);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_F32,   0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,   GX_F32,   0);
}

/*---------------------------------------------------------------------------*
    Name:			DrawTick
    
    Description:	Draws spheres
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static void DrawTick( void )
{
    u32 i, j, k, cnt; // indices
    u32 tx;  // offset of texture
    Mtx m, t;
    f32 x, z;

    // for now do only 2-D grid
    // center grid on 0, 0, 0
    z = 0.0F - ((testData.numz / 2) * testData.dz);
    for (cnt = i = 0; i < testData.numz; i++)
    {
        x = 0.0F - ((testData.numx / 2) * testData.dx);
        for (j = 0; j < testData.numx; j++, cnt++)
        {
            // Set texture state every n cubes
            if ((cnt % testData.nc_tx) == 0)
            {
                tx = ((cnt / testData.nc_tx) & 1) * (NUM_TEXOBJS - 1);
                for (k = 0; k < testData.ntev; k++)
                {
                    if (tx)
                        GXLoadTexObj(&texObj[tx], (GXTexMapID)k);
                    else
                        GXLoadTexObj(&texObj[k], (GXTexMapID)k);
                }
            }
            // build matrix
	        MTXScale(m, testData.mods, testData.mods, testData.mods);
	        MTXRotAxisDeg(t, &testData.axis[cnt%NUM_ROTMATS], testData.rot);
	        MTXConcat(t, m, m);  // scale, rotate
	        MTXTrans(t, x, 0.0F, z);
	        MTXConcat(t, m, m);  // translate
	        MTXConcat(v, m, mv); // concat view matrix

	        GXLoadPosMtxImm(mv, 0);
	        GXLoadNrmMtxImm(mv, 0);

            testDraw();

            x += testData.dx;
        }
        z += testData.dz;
    }

    // draw arrow on top-center
	MTXScale(m, testData.mods, testData.mods, testData.mods);
	MTXTrans(t, 0.0F, -100.0F*testData.mods, 0.0F);
	MTXConcat(t, m, m);  // scale, translate
	MTXConcat(v, m, mv); // concat view matrix

	GXLoadPosMtxImm(mv, 0);
    DrawArrow();
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Animates the camera and object based on the joystick's 
                    state.
                    
    Arguments:      v    view matrix
                    m    model matrix
                    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick ( void )
{
    Mtx rot;
    Vec temp;
    Vec yAxis = {0.0F, 1.0F, 0.0F};
    f32 dot;

    // Get controller status through DEMOPad library
    u16 buttons = DEMOPadGetButton(0);
    u16 downs   = DEMOPadGetButtonDown(0);
    u16 dirs    = DEMOPadGetDirsNew(0);
    s8 stickX   = DEMOPadGetStickX(0);
    s8 stickY   = DEMOPadGetStickY(0);

    // Scale model up/down
    if (buttons & PAD_TRIGGER_L)
    {
        testData.mods *= 0.95F;
        if (testData.mods < 0.001F)
            testData.mods = 0.001F;
    }
    if (buttons & PAD_TRIGGER_R)
    {
        testData.mods *= 1.05F;
    }

    // Model rotation
    testData.rot += testData.dr;
    if (testData.rot >= 360.0F)
        testData.rot -= 360.0F;

    // change number of cubes
    if (dirs & DEMO_SUBSTICK_UP)
    {
        testData.numz++;
    }
    if (dirs & DEMO_SUBSTICK_DOWN)
    {
        testData.numz--;
        if (testData.numz <= 1)
            testData.numz = 1;
    }
    if (dirs & DEMO_SUBSTICK_RIGHT)
    {
        testData.numx++;
    }
    if (dirs & DEMO_SUBSTICK_LEFT)
    {
        testData.numx--;
        if (testData.numx <= 1)
            testData.numx = 1;
    }

    // Zoom camera in/out
    if(buttons & PAD_BUTTON_Y)
    {
        CamS *= .95F;
        if(CamS < 0.001F)
        {
            CamS = 0.001F;
        }
    }
    if(buttons & PAD_BUTTON_A)
    {
        CamS *= 1.05F;         
    }

    // Rotate viewpoint
    if(stickX || stickY)
    {
        if(stickX > 30 || stickX < -30)
        { 
            if(stickX > 30) 
                MTXRotDeg(rot, 'y', -1.0F);
            else if(stickX < -30)
                MTXRotDeg(rot, 'y', 1.0F);

            MTXMultVec(rot, &CamX, &CamX);
            MTXMultVec(rot, &CamY, &CamY); 
            MTXMultVec(rot, &CamZ, &CamZ); 
        }

        if(stickY > 30 || stickY < -30)
        { 
            if(stickY > 30)
                MTXRotAxisDeg(rot, &CamX, -1.0F);
            else if(stickY < -30)
                MTXRotAxisDeg(rot, &CamX, 1.0F);

            MTXMultVec(rot, &CamY, &temp);
            dot = VECDotProduct(&temp, &yAxis);
            if(dot > 0.05F && dot <= 1.0F)
            {
                CamY = temp;
                MTXMultVec(rot, &CamZ, &CamZ);
            }
        }
    }

    // change mode
    if(downs & PAD_BUTTON_X)
    {
        animMode++;
        animMode %= 6;

        switch (animMode)
        {
            case 0:
                OSReport("Button B increments the number of tev stages\n");
                break;
            case 1:
                OSReport("Button B toggles the statistics display\n"); 
                break;
            case 2:
                OSReport("Button B toggles the cube animation\n"); 
                break;
            case 3:
                OSReport("Button B decrements the number of cubes per texture\n"); 
                break;
            case 4:
                OSReport("Button B toggles the texture cache algorithm\n"); 
                break;
            case 5:
                OSReport("Button B toggles texture invalidation mode\n"); 
                break;
            default:
                break;
        }
    } 

    // change param depending on mode
    if (downs & PAD_BUTTON_B)
    {
        if (animMode == 0)
        {
            testData.ntev++;
            if (testData.ntev > MAX_TEV)
                testData.ntev = 1;

        } 
        else if (animMode == 1) 
        {
            showStats++;
            switch(showStats)
            {
              case 1 :
                DEMOSetStats(MyStats1, STAT_MENU1, DEMO_STAT_TLD);
                break;
              case 2 :
                DEMOSetStats(MyStats2, STAT_MENU2, DEMO_STAT_TLD);
                break;
              case 3 :
                DEMOSetStats(MyStats3, STAT_MENU3, DEMO_STAT_TLD);
                break;
              case 4 :
                DEMOSetStats(MyStats4, STAT_MENU4, DEMO_STAT_TLD);
                break;
              case 5 :
                DEMOSetStats(MyStats5, STAT_MENU5, DEMO_STAT_TLD);
                break;
              default:
                DEMOSetStats(NULL, STAT_MENU0, DEMO_STAT_TLD);
                showStats = 0;
                break;
            }
        }
        else if (animMode == 2) 
        {
            stopAnim ^= 1;
            if (stopAnim)
                testData.dr   = 0.0F;   // rotation speed
            else
                testData.dr   = 1.0F;   // rotation speed
        }
        else if (animMode == 3) 
        {
            testData.nc_tx /= 2;
            if (testData.nc_tx == 0)
                testData.nc_tx = testData.numx * testData.numz; 
        }
        else if (animMode == 4) 
        {
            testData.tcb ^= 1;
            if (testData.tcb == 0)
            {
                OSReport("Texture cache = round robin\n");
                GXSetTexRegionCallback(oldCallback);
            }
            else
            {
                OSReport("Texture cache = mapID\n");
                oldCallback = GXSetTexRegionCallback(MyTexRegionCallback);
            }
            GXInvalidateTexAll();
        }
        else if (animMode == 5) 
        {
            testData.tci ^= 1;
            if (testData.tci && testData.tcb)
                OSReport("Invalidate tex region on each GXLoadTexObj\n");
            else
                OSReport("Do not invalidate tex region on each GXLoadTexObj\n");
        }
    }
}


/*---------------------------------------------------------------------------*
    Name:			MakeModelMtx
    
    Description:	computes a model matrix from 3 vectors representing an 
					object's coordinate system.
    				
    Arguments:		xAxis	vector for the object's X axis
					yAxis	vector for the object's Y axis
					zAxis	vector for the object's Z axis
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static void MakeModelMtx ( Vec xAxis, Vec yAxis, Vec zAxis, Mtx m )
{
	VECNormalize(&xAxis,&xAxis);
	VECNormalize(&yAxis,&yAxis);
	VECNormalize(&zAxis,&zAxis);

	m[0][0] = xAxis.x;
	m[0][1] = xAxis.y;
	m[0][2] = xAxis.z;
	m[0][3] = 0.0F;

	m[1][0] = yAxis.x;
	m[1][1] = yAxis.y;
	m[1][2] = yAxis.z;
	m[1][3] = 0.0F;

	m[2][0] = zAxis.x;
	m[2][1] = zAxis.y;
	m[2][2] = zAxis.z;
	m[2][3] = 0.0F;

	MTXInverse(m, m);
}


/*---------------------------------------------------------------------------*
    Name:           MyTexRegionCallback
    
    Description:    Tex cache allocator using simple round algorithm

    Arguments:      texObj : a pointer to texture object to be loaded
                    mapID  : destination texmap ID (just same as GXLoadTexObj)
    
    Returns:        appropriate tex cache region for loading texture.
 *---------------------------------------------------------------------------*/
static GXTexRegion* MyTexRegionCallback(GXTexObj* texObj, GXTexMapID mapID)
{
#pragma unused(mapID)

    u32  regionNum, texID;
   
    texID     = (u32)GXGetTexObjUserData(texObj);
    regionNum = texID % NUM_REGIONS;

    // see effect of cold cache load
    if (testData.tci)
        GXInvalidateTexRegion(&MyTexRegions[regionNum]);

    return &MyTexRegions[regionNum];
}



/*---------------------------------------------------------------------------*
    Name:			PrintIntro
    
    Description:	Print usage for test
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n");
    OSReport("************************************\n");
    OSReport(" Check fill-rate performance\n");
    OSReport("************************************\n");
    OSReport("\n");
    OSReport("SUBSTICK:   Change the number of cubes\n");
    OSReport("TRIGGER R:  Increase the size of the cubes\n");
    OSReport("TRIGGER L:  Decrease the size of the cubes\n");
    OSReport("\n");
    OSReport("STICK:      Rotate camera\n");
    OSReport("BUTTON A/Y: Zoom camera in/out\n");
    OSReport("BUTTON X:   Change Button B mode\n");
    OSReport("BUTTON B:   Controls various parameters depending on mode\n");
    OSReport("\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
}


/*---------------------------------------------------------------------------*
    Name:			DrawCubeIndx
    
    Description:	
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static
void DrawCubeIndx( void )
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);
		GXPosition1x16(4);
        GXColor1x8(0);
        GXTexCoord1x8(0); //
		GXPosition1x16(5);
        GXColor1x8(0);
        GXTexCoord1x8(1); // 
		GXPosition1x16(6);
        GXColor1x8(0);
        GXTexCoord1x8(2); // 
		GXPosition1x16(7);			
        GXColor1x8(0);
        GXTexCoord1x8(3); // 

		GXPosition1x16(2);
        GXColor1x8(1);
        GXTexCoord1x8(0); //
		GXPosition1x16(6);
        GXColor1x8(1);
        GXTexCoord1x8(1); //
		GXPosition1x16(5);
        GXColor1x8(1);
        GXTexCoord1x8(2); //
		GXPosition1x16(3);
        GXColor1x8(1);
        GXTexCoord1x8(3); //
	
		GXPosition1x16(1);
        GXColor1x8(2);
        GXTexCoord1x8(0); //
		GXPosition1x16(0);
        GXColor1x8(2);
        GXTexCoord1x8(1); //
		GXPosition1x16(4);
        GXColor1x8(2);
        GXTexCoord1x8(2); //
		GXPosition1x16(7);
        GXColor1x8(2);
        GXTexCoord1x8(3); //
   
 		GXPosition1x16(0);
        GXColor1x8(3);
        GXTexCoord1x8(0); //
		GXPosition1x16(1);
        GXColor1x8(3);
        GXTexCoord1x8(1); //
		GXPosition1x16(2);
        GXColor1x8(3);
        GXTexCoord1x8(2); //
		GXPosition1x16(3);
        GXColor1x8(3);
        GXTexCoord1x8(3); //
      
        GXPosition1x16(5);
        GXColor1x8(4);
        GXTexCoord1x8(0); //
        GXPosition1x16(4);
        GXColor1x8(4);
        GXTexCoord1x8(1); //
        GXPosition1x16(0);
        GXColor1x8(4);
        GXTexCoord1x8(2); //
        GXPosition1x16(3);
        GXColor1x8(4);
        GXTexCoord1x8(3); //
	 		
	 	GXPosition1x16(6);
        GXColor1x8(5);
        GXTexCoord1x8(0); //
	 	GXPosition1x16(2);
        GXColor1x8(5);
        GXTexCoord1x8(1); //
	 	GXPosition1x16(1);
        GXColor1x8(5);
        GXTexCoord1x8(2); //
	 	GXPosition1x16(7);
        GXColor1x8(5);
        GXTexCoord1x8(3); //

	GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:			DrawCubeDirect
    
    Description:	
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static
void DrawCubeDirect( void )
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(0.0F, 1.0F);//

		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(0.0F, 1.0F);//
	
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(0.0F, 1.0F);//
   
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(0.0F, 1.0F);//
      
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(0.0F, 1.0F);//
	 		
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(0.0F, 1.0F);//
	GXEnd();
}


/*---------------------------------------------------------------------------*
    Name:			DrawArrow
    
    Description:	Draw arrow pointing in direction of Z sort (-Z axis)
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static void DrawArrow( void )
{
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS,  GX_INDEX16);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);

	GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3*4);
        // tri 0
		GXPosition1x16(10);
        GXColor1x8(1);// green

		GXPosition1x16(8);
        GXColor1x8(1);// green

		GXPosition1x16(11);
        GXColor1x8(1);// green


        // tri 1
		GXPosition1x16(10);
        GXColor1x8(0);// red

		GXPosition1x16(11);
        GXColor1x8(0);// red

		GXPosition1x16(9);
        GXColor1x8(0);// red


        // tri 2
		GXPosition1x16(9);
        GXColor1x8(1);// green

		GXPosition1x16(11);
        GXColor1x8(1);// green

		GXPosition1x16(8);
        GXColor1x8(1);// green


        // tri 3
		GXPosition1x16(10);
        GXColor1x8(1);// green

		GXPosition1x16(9);
        GXColor1x8(1);// green

		GXPosition1x16(8);
        GXColor1x8(1);// green
    GXEnd();
}

//
//   My random functions
//
#define MAX_RAND	(0xffffffff)
#define SEED        (0x38ba990e)

static
void init_rand( void )
{
    srand(SEED);    
}


static
u32 lrand( void )
{
    u32	r;
    r = (u32) ((rand() & 0xffff) << 16) + (rand() & 0xffff);
    return(r);
}


static
f32 frand( void )
{
    f32	f;
    f = (f32)lrand()/(f32)MAX_RAND;
    return (f);
}
