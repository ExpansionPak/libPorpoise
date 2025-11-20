/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     imm-perf.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  Immediate-mode performance test

  $Log: /Dolphin/build/demos/gxdemo/src/Performance/perf-geo.c $
    
    4     5/13/01 2:55p Hirose
    removed obsolete flags
    
    3     8/23/00 2:53p Alligator
    allow perf counter api to work with draw sync callback
    
    2     7/01/00 2:52p Alligator
    intro stuff
    
    1     6/20/00 11:55a Alligator
    initial performance demos
    
    6     6/19/00 1:43p Alligator
    added texture-only cases, added normal data to tex+lit case
    
    5     6/12/00 4:13p Alligator
    commented out fifo pointer checking code
    
    4     6/12/00 1:46p Alligator
    updated demo statistics to support new api
    
    3     6/08/00 3:54p Alligator
    added indexed/direct switch, added all perf0 counters
    
    2     6/06/00 6:57p Alligator
    added vcache mem and stall counters
    
    1     6/06/00 12:05p Alligator
    immediate mode performance test
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main             ( void );
static void CameraInit       ( void );
static void DrawInit         ( void );
static void DrawTick         ( void );
static void DumpStats        ( void );
static void PrintIntro       ( void );
static u32  checkForOverflow ( void );
static void DrawCubeIndx     ( void );
static void DrawCubeDirect   ( void );
static void DrawTexCubeIndx  ( void );
static void DrawTexCubeDirect( void );


/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/
#define BALL64_TEX_ID   8
#define N_TESTS         (sizeof(testData)/sizeof(TestData))
#define N_STATS         (sizeof(Stat)/sizeof(StatObj))
#define SIDE            50.0F

typedef enum {
    GP_PERF0,
    GP_PERF1,
    MEM_PERF,
    PIX_PERF,
    VC_PERF
} PerfType;

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
//
//  Cube data
// 
f32 Vert[] ATTRIBUTE_ALIGN(32) =  
                     {  
                        -SIDE, SIDE, -SIDE, //0
                        -SIDE, SIDE, SIDE,  //1
                        -SIDE, -SIDE, SIDE, //2
                        -SIDE, -SIDE, -SIDE,//3
                        SIDE, SIDE, -SIDE,  //4
                        SIDE, -SIDE, -SIDE, //5
                        SIDE, -SIDE, SIDE,  //6
                        SIDE, SIDE, SIDE    //7
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

// index when lsb=1
typedef enum {
    CUBE_DIR,     // lit and textured
    CUBE_IDX,     // lit and textured
    CUBE_TEX_DIR, // textured only
    CUBE_TEX_IDX  // textured only
} CubeType;

//
//  Test cases
//
typedef struct {
    u32      nloops;  // number of loops (ncubes + nmtx)
    u32      ncubes;  // number of cubes to draw, 24 verts each
    u32      nmtx;    // number of matrix loads per ncubes
    u32      fill;    // fill FIFO flag, if true fill FIFO before rendering
    CubeType type;    // type of vertex data
} TestData;

static TestData testData[] = 
{  //  nloops, ncubes, nmtx, fill, type

{       1,      0, 1000,    1,    CUBE_DIR }, // test matrix load speed
{       1,   1000,    1,    1,    CUBE_DIR }, // test a few large objects
{    1000,      1,    1,    1,    CUBE_DIR }, // test very small objects 
{     500,      2,    1,    1,    CUBE_DIR }, // 
{     333,      3,    1,    1,    CUBE_DIR }, // 
{     250,      4,    1,    1,    CUBE_DIR }, // 
{     200,      5,    1,    1,    CUBE_DIR }, // 
{     100,     10,    1,    1,    CUBE_DIR }, // test many small objects
{     100,     10,    2,    1,    CUBE_DIR }, // how does n mtx effect?
{     100,     10,    3,    1,    CUBE_DIR },
{     100,     10,   10,    1,    CUBE_DIR },

{       1,      0, 1000,    1,    CUBE_IDX }, // test matrix load speed
{       1,   1000,    1,    1,    CUBE_IDX }, // test a few large objects
{    1000,      1,    1,    1,    CUBE_IDX }, // test very small objects
{     500,      2,    1,    1,    CUBE_IDX }, // 
{     333,      3,    1,    1,    CUBE_IDX }, // 
{     250,      4,    1,    1,    CUBE_IDX }, // 
{     200,      5,    1,    1,    CUBE_IDX }, // 
{     100,     10,    1,    1,    CUBE_IDX }, // test many small objects
{     100,     10,    2,    1,    CUBE_IDX }, // how does n mtx effect?
{     100,     10,    3,    1,    CUBE_IDX },
{     100,     10,   10,    1,    CUBE_IDX },

{       1,      0, 1000,    1,    CUBE_TEX_DIR }, // test matrix load speed
{       1,   1000,    1,    1,    CUBE_TEX_DIR }, // test a few large objects
{    1000,      1,    1,    1,    CUBE_TEX_DIR }, // test very small objects 
{     500,      2,    1,    1,    CUBE_TEX_DIR }, // 
{     333,      3,    1,    1,    CUBE_TEX_DIR }, // 
{     250,      4,    1,    1,    CUBE_TEX_DIR }, // 
{     200,      5,    1,    1,    CUBE_TEX_DIR }, // 
{     100,     10,    1,    1,    CUBE_TEX_DIR }, // test many small objects
{     100,     10,    2,    1,    CUBE_TEX_DIR }, // how does n mtx effect?
{     100,     10,    3,    1,    CUBE_TEX_DIR },
{     100,     10,   10,    1,    CUBE_TEX_DIR },

{       1,      0, 1000,    1,    CUBE_TEX_IDX }, // test matrix load speed
{       1,   1000,    1,    1,    CUBE_TEX_IDX }, // test a few large objects
{    1000,      1,    1,    1,    CUBE_TEX_IDX }, // test very small objects
{     500,      2,    1,    1,    CUBE_TEX_IDX }, // 
{     333,      3,    1,    1,    CUBE_TEX_IDX }, // 
{     250,      4,    1,    1,    CUBE_TEX_IDX }, // 
{     200,      5,    1,    1,    CUBE_TEX_IDX }, // 
{     100,     10,    1,    1,    CUBE_TEX_IDX }, // test many small objects
{     100,     10,    2,    1,    CUBE_TEX_IDX }, // how does n mtx effect?
{     100,     10,    3,    1,    CUBE_TEX_IDX },
{     100,     10,   10,    1,    CUBE_TEX_IDX } 
};

//
//  Statistics
//
typedef struct {
    u32         cnt;  
    PerfType    stat_type;
    u32         stat;
    char        text[50];
} StatObj;

StatObj Stat[] = { // stats to count
    { 0, GP_PERF0, GX_PERF0_VERTICES,         "vertices..................." },
//    { 0, GP_PERF0, GX_PERF0_CLIP_VTX,         "num vtx clipped............" },
//    { 0, GP_PERF0, GX_PERF0_CLIP_CLKS,        "num clipping clocks........" },
    { 0, GP_PERF0, GX_PERF0_XF_WAIT_IN,       "xf waiting on input........" },
    { 0, GP_PERF0, GX_PERF0_XF_WAIT_OUT,      "xf waiting on output......." },
    { 0, GP_PERF0, GX_PERF0_XF_XFRM_CLKS,     "xf transform clocks........" },
    { 0, GP_PERF0, GX_PERF0_XF_LIT_CLKS,      "xf lighting clocks........." },
    { 0, GP_PERF0, GX_PERF0_XF_BOT_CLKS,      "xf bot of pipe clocks......" },
    { 0, GP_PERF0, GX_PERF0_XF_REGLD_CLKS,    "xf register load clocks...." },
    { 0, GP_PERF0, GX_PERF0_XF_REGRD_CLKS,    "xf register read clocks...." },
//    { 0, GP_PERF0, GX_PERF0_CLIP_RATIO,       "clip ratio................." },
    { 0, GP_PERF0, GX_PERF0_TRIANGLES,        "Triangles.................." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_CULLED, "Triangles culled..........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_PASSED, "Triangles !culled.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_SCISSORED, "Triangles scissored........" },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_0TEX,   "Triangles w/no tex........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_1TEX,   "Triangles w/1 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_2TEX,   "Triangles w/2 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_3TEX,   "Triangles w/3 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_4TEX,   "Triangles w/4 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_5TEX,   "Triangles w/5 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_6TEX,   "Triangles w/6 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_7TEX,   "Triangles w/7 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_8TEX,   "Triangles w/8 tex.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_0CLR,   "Triangles w/no clr........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_1CLR,   "Triangles w/1 clr.........." },
//    { 0, GP_PERF0, GX_PERF0_TRIANGLES_2CLR,   "Triangles w/2 clr.........." },
//    { 0, GP_PERF0, GX_PERF0_QUAD_0CVG,        "Quads w/0 coverage........." },
//    { 0, GP_PERF0, GX_PERF0_QUAD_NON0CVG,     "Quads w/>0 coverage........" },
//    { 0, GP_PERF0, GX_PERF0_QUAD_1CVG,        "Quads w/1 pix coverage....." },
//    { 0, GP_PERF0, GX_PERF0_QUAD_2CVG,        "Quads w/2 pix coverage....." },
//    { 0, GP_PERF0, GX_PERF0_QUAD_3CVG,        "Quads w/3 pix coverage....." },
//    { 0, GP_PERF0, GX_PERF0_QUAD_4CVG,        "Quads w/4 pix coverage....." },
//    { 0, GP_PERF0, GX_PERF0_AVG_QUAD_CNT,     "Average quad count........." },
    { 0, GP_PERF0, GX_PERF0_CLOCKS,           "Perf0 clocks..............." },

//    { 0, GP_PERF1, GX_PERF1_TEXELS,           "Number of texels..........." },
//    { 0, GP_PERF1, GX_PERF1_TX_IDLE,          "Tex idle..................." },
//    { 0, GP_PERF1, GX_PERF1_TX_REGS,          "Tex regs..................." },
//    { 0, GP_PERF1, GX_PERF1_TX_MEMSTALL,      "Tex mem stall.............." },
//    { 0, GP_PERF1, GX_PERF1_TC_CHECK1_2,      "Tex$ check 1,2............." },
//    { 0, GP_PERF1, GX_PERF1_TC_CHECK3_4,      "Tex$ check 3,4............." },
//    { 0, GP_PERF1, GX_PERF1_TC_CHECK5_6,      "Tex$ check 5,6............." },
//    { 0, GP_PERF1, GX_PERF1_TC_CHECK7_8,      "Tex$ check 7,8............." },
//    { 0, GP_PERF1, GX_PERF1_TC_MISS,          "Tex$ miss.................." },
    { 0, GP_PERF1, GX_PERF1_VC_ELEMQ_FULL,    "Vtx$ elem queue full......." },
    { 0, GP_PERF1, GX_PERF1_VC_MISSQ_FULL,    "Vtx$ miss queue full......." },
    { 0, GP_PERF1, GX_PERF1_VC_MEMREQ_FULL,   "Vtx$ mem request full......" },
    { 0, GP_PERF1, GX_PERF1_VC_STATUS7,       "Vtx$ status 7.............." },
    { 0, GP_PERF1, GX_PERF1_VC_MISSREP_FULL,  "Vtx$ miss replace full....." },
    { 0, GP_PERF1, GX_PERF1_VC_STREAMBUF_LOW, "Vtx$ stream buf low........" },
    { 0, GP_PERF1, GX_PERF1_VC_ALL_STALLS,    "Vtx$ all stalls............" },
    { 0, GP_PERF1, GX_PERF1_VERTICES,         "Perf1 vertices............." },
    { 0, GP_PERF1, GX_PERF1_FIFO_REQ,         "CP fifo requests..........." },
    { 0, GP_PERF1, GX_PERF1_CALL_REQ,         "CP call requests..........." },
    { 0, GP_PERF1, GX_PERF1_VC_MISS_REQ,      "Vtx$ miss request.........." },
    { 0, GP_PERF1, GX_PERF1_CP_ALL_REQ,       "CP all requests............" },
    { 0, GP_PERF1, GX_PERF1_CLOCKS,           "Perf1 clocks..............." },
    { 0, MEM_PERF, 0,                         "Memory requests............" }
//    { 0, PIX_PERF, 0,                         "Pixel stats................" },
//    { 0, VC_PERF,  0,                         "VCache stats..............." } 
};

//
//  Global vars
//
static Mtx v, mv;

static u32 Test    = 0;
static u32 curStat = 0;
static void (*testDraw)() = NULL; // function pointer for draw routine

// pixel statistics counters
static u32 topPixIn;
static u32 topPixOut;
static u32 botPixIn;
static u32 botPixOut;
static u32 clrPixIn;
static u32 copyClks;
// vcache statistics counters
static u32 vcCheck;
static u32 vcMiss;
static u32 vcStall;
// clocks per verts
static u32 cpReq,
           tcReq,
           cpuRdReq,
           cpuWrReq,
           dspReq,
           ioReq,
           viReq,
           peReq,
           rfReq,
           fiReq;

static OSStopwatch myTotTime;
static OSStopwatch myCpuTime;
static OSStopwatch myGpTime;
static GXFifoObj newFifo;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    u32 fillFifo = 0;
    u32 i        = 0;
    GXFifoObj* waitFifo = NULL; // this is the default GX fifo
    //void *readPtr0, *writePtr0;
    //void *readPtr1, *writePtr1;


    DEMOInit(NULL);
    DrawInit(); 
    PrintIntro();

    OSInitStopwatch(&myTotTime, "myTotTime");
    OSInitStopwatch(&myCpuTime, "myCpuTime");
    OSInitStopwatch(&myGpTime, "myGpTime");

    // Allocate memory initialize the newFifo.  This fifo is used to hold
    // a complete 'frame' to draw.  We don't really need a complete frame,
    // just enough to see the performance difference.
    GXInitFifoBase(&newFifo, 
                   OSAlloc(sizeof(u8)*1000*1500), sizeof(u8)*1000*1500);

    // get the default fifo, this will be emptied and then used
    // to keep the GP from reading when we select fillFifo mode
    waitFifo = GXGetCPUFifo();
    Test = 0;

    while (Test != N_TESTS)
    {
        OSResetStopwatch(&myTotTime);
        OSResetStopwatch(&myCpuTime);
        OSResetStopwatch(&myGpTime);

        fillFifo = testData[Test].fill;
        curStat = 0;
        while (curStat != N_STATS)
        {
            DEMOBeforeRender();

            //
            //  Enable various statistics
            //  clear counters
            //
            if (Stat[curStat].stat_type == GP_PERF0)
            {
                GXSetGP0Metric((GXPerf0)Stat[curStat].stat);
                GXClearGP0Metric();
            }
            else if (Stat[curStat].stat_type == GP_PERF1)
            {
                GXSetGP1Metric((GXPerf1)Stat[curStat].stat);
                GXClearGP1Metric();
            }
            else if (Stat[curStat].stat_type == MEM_PERF)
            {
                GXClearMemMetric();
            }
            else if (Stat[curStat].stat_type == PIX_PERF)
            {
                GXClearPixMetric();
            }
            else // vcache stat
            {
                GXSetVCacheMetric(GX_VC_ALL);
                GXClearVCacheMetric();
            }

            // make sure all GX commands to here are flushed
            GXSaveCPUFifo(GXGetCPUFifo());
            //GXGetFifoPtrs(GXGetCPUFifo(), &readPtr0, &writePtr0);
            //OSReport("Write pointer, before loop: %08x\n", writePtr0);

            // choose normal immediate mode or delayed immediate mode
            if (fillFifo)
            {
                // set new FIFO
                if (GXGetCPUFifo() != &newFifo)
                {
                    // reset read/write pointers
                    GXInitFifoPtrs(&newFifo, 
                                    GXGetFifoBase(&newFifo), 
                                    GXGetFifoBase(&newFifo));
                    GXSetCPUFifo(&newFifo);
                }
                OSStartStopwatch(&myCpuTime);
            } 
            else
            {
                // reset read write pointers
                if (GXGetCPUFifo() != waitFifo)
                {
                    GXInitFifoPtrs(waitFifo, 
                                   GXGetFifoBase(waitFifo), 
                                   GXGetFifoBase(waitFifo));
                    // set new FIFO
                    GXSetCPUFifo(waitFifo);
                }
            }

            OSStartStopwatch(&myTotTime);


            DrawTick();

            // turn on FIFO reads after whole FIFO is filled
            if (fillFifo)
            {
                // push all data out
                GXFlush();
                OSStopStopwatch(&myCpuTime); // CPU is done here

                // make sure we are in the newFifo limits
                if (checkForOverflow())
                    OSHalt("Error, FIFO overflowed");

                // switch to new fifo
                if (GXGetGPFifo() != GXGetCPUFifo())
                    GXSetGPFifo(GXGetCPUFifo());

                OSStartStopwatch(&myGpTime); // GP starts here
            }

            GXDrawDone();   // wait for GP to be done
                            // GXDrawDone does flush too
                            
            //GXGetFifoPtrs(GXGetCPUFifo(), &readPtr1, &writePtr1);
            //OSReport("Write pointer, after loop:  %08x\n", writePtr1);
            //OSReport("Diff:  %8d (cache lines)\n", 
            //        ((u32)writePtr1 - (u32)writePtr0) / 32);

            OSStopStopwatch(&myTotTime);

            //
            //  Read back various statistics
            //
            if (Stat[curStat].stat_type == GP_PERF0)
            {
                Stat[curStat].cnt = GXReadGP0Metric();
            }
            else if (Stat[curStat].stat_type == GP_PERF1)
            {
                Stat[curStat].cnt = GXReadGP1Metric();
            }
            else if (Stat[curStat].stat_type == MEM_PERF)
            {
                GXReadMemMetric   ( &cpReq,
                                    &tcReq,
                                    &cpuRdReq,
                                    &cpuWrReq,
                                    &dspReq,
                                    &ioReq,
                                    &viReq,
                                    &peReq,
                                    &rfReq,
                                    &fiReq );
            }
            else if (Stat[curStat].stat_type == PIX_PERF)
            {
                GXReadPixMetric(&topPixIn, &topPixOut, &botPixIn, &botPixOut,
                                &clrPixIn, &copyClks);
            }
            else // vcache stats
            {
                GXReadVCacheMetric(&vcCheck, &vcMiss, &vcStall);
            }

            // turn off FIFO reads.
            if (fillFifo)
            {
                OSStopStopwatch(&myGpTime);
                // GP fifo is already empty, waitFifo is empty and no one
                // will write to it when fillFifo is true.  This is how
                // we keep the GP waiting until newFifo is ready.
                if (GXGetGPFifo() != waitFifo)
                    GXSetGPFifo(waitFifo);
                // make cpu fifo = wait fifo, so next time through we can
                // reset the pointer to the top.
                if (GXGetCPUFifo() != waitFifo)
                    GXSetCPUFifo(waitFifo);
            }

            DEMODoneRender();
            curStat++;
        }

        DumpStats();
        Test++;
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
static 
void CameraInit      ( void )
{
    Mtx44 p;
    Vec camPt  = {0.0F, 0.0F, 650.0F};
    Vec up     = {0.0F, 1.0F, 0.0F};
    Vec origin = {0.0F, 0.0F, -100.0F};
    
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
static 
void DrawInit( void )
{
    GXLightObj MyLight;
    GXColor color = {0, 255, 255, 255};
    GXTexObj      texObj;  // texture object
    TEXPalettePtr tpl = 0; // texture palette 

#ifndef  __SINGLEFRAME
    // test max geometry transform performance
    GXSetCullMode(GX_CULL_ALL);
    //GXSetCullMode(GX_CULL_BACK);
#endif // __SINGLEFRAME

    // constant matrix, just want to see load performance
    CameraInit();   // Initialize the camera (v)
	MTXTrans(mv, 0, 0, 0);
	MTXConcat(v, mv, mv);

    GXSetCurrentMtx(0);

    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, color);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);

    GXSetChanMatColor(GX_COLOR0A0, color);
    //  Load the texture palette
    TEXGetPalette(&tpl, "gxTextrs.tpl");
    //  Initialize a texture object to contain the correct texture
    TEXGetGXTexObjFromPalette(tpl, &texObj, BALL64_TEX_ID);
    //  Load the texture object; tex0 is used in stage 0
    GXLoadTexObj(&texObj, GX_TEXMAP0);

    GXSetNumTevStages(1);

    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

    GXSetArray(GX_VA_POS,  Vert,     sizeof(f32)*3);
    GXSetArray(GX_VA_NRM,  Norm,     sizeof(f32)*3);
    GXSetArray(GX_VA_CLR0, MyColors, sizeof(u32));
    GXSetArray(GX_VA_TEX0, TexCoord, sizeof(f32)*2);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    //GXDrawDone(); // make sure state is in GP
}



/*---------------------------------------------------------------------------*
    Name:			DrawTick
    
    Description:	Draws cubes
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static
void DrawTick( void )
{
    u32 i, j; // indices

    if (testData[Test].type == CUBE_IDX)
    {
        //OSReport("Using indexed data, tex and lit\n");
        GXSetNumChans(1);
        GXSetNumTexGens(1);

        GXSetChanCtrl(
                GX_COLOR0A0,
                GX_ENABLE,      // enable channel
                GX_SRC_REG,     // amb source
                GX_SRC_VTX,     // mat source
                GX_LIGHT0,      // light mask
                GX_DF_NONE,     // diffuse function
                GX_AF_NONE);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS,  GX_INDEX16);
        GXSetVtxDesc(GX_VA_NRM,  GX_INDEX8);
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
        testDraw = DrawCubeIndx;
    }
    else if (testData[Test].type == CUBE_DIR)
    {
        //OSReport("Using direct data, tex and lit\n");
        GXSetNumChans(1);
        GXSetNumTexGens(1);

        GXSetChanCtrl(
                GX_COLOR0A0,
                GX_ENABLE,      // enable channel
                GX_SRC_REG,     // amb source
                GX_SRC_VTX,     // mat source
                GX_LIGHT0,      // light mask
                GX_DF_NONE,     // diffuse function
                GX_AF_NONE);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
        GXSetVtxDesc(GX_VA_NRM,  GX_DIRECT);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        testDraw = DrawCubeDirect;
    }
    else if (testData[Test].type == CUBE_TEX_IDX)
    {
        //OSReport("Using indexed data, tex-only\n");
        GXSetNumChans(0);
        GXSetNumTexGens(1);

        GXSetChanCtrl(
                GX_COLOR0A0,
                GX_DISABLE,     // enable channel
                GX_SRC_REG,     // amb source
                GX_SRC_REG,     // mat source
                GX_LIGHT0,      // light mask
                GX_DF_NONE,     // diffuse function
                GX_AF_NONE);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS,  GX_INDEX16);
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
        testDraw = DrawTexCubeIndx;
    }
    else if (testData[Test].type == CUBE_TEX_DIR)
    {
        //OSReport("Using direct data, tex-only\n");
        GXSetNumChans(0);
        GXSetNumTexGens(1);

        GXSetChanCtrl(
                GX_COLOR0A0,
                GX_DISABLE,     // enable channel
                GX_SRC_REG,     // amb source
                GX_SRC_REG,     // mat source
                GX_LIGHT0,      // light mask
                GX_DF_NONE,     // diffuse function
                GX_AF_NONE);

        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        testDraw = DrawTexCubeDirect;
    }


    for (i = 0; i < testData[Test].nloops; i++)
    {
        for (j = 0; j < testData[Test].nmtx; j++)
        {
	        GXLoadPosMtxImm(mv, 0);
	        GXLoadNrmMtxImm(mv, 0);
        }
        for (j = 0; j < testData[Test].ncubes; j++)
        {
            testDraw();
        }
    }
}

/*---------------------------------------------------------------------------*
    Name:			DumpStats
    
    Description:	Print current counters to stdout
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static
void DumpStats( void )
{
    u32 i;

        OSReport("******** Test %3d ********\n", Test);

        if (testData[Test].fill)
            OSReport("Fill FIFO before rendering\n");
        else
            OSReport("Write to FIFO while rendering\n");

        switch (testData[Test].type)
        {
            case CUBE_DIR: OSReport("Direct data, tex and lit\n"); break;
            case CUBE_IDX: OSReport("Indirect data, tex and lit\n"); break;
            case CUBE_TEX_DIR: OSReport("Direct data, tex only\n"); break;
            case CUBE_TEX_IDX: OSReport("Indirect data, tex only\n"); break;
        }

        OSReport("Number of loops.......... %8d\n", testData[Test].nloops);
        OSReport("Number of cubes.......... %8d\n", testData[Test].ncubes);
        OSReport("Number of matrices/cube.. %8d\n", testData[Test].nmtx);

        for (i = 0; i < N_STATS; i++)
        {
            if (Stat[i].stat_type == PIX_PERF)
            {
                OSReport("Top pixels in..............:   %8d\n", topPixIn);
                OSReport("Top pixels out.............:   %8d\n", topPixOut);
                OSReport("Bot pixels in..............:   %8d\n", botPixIn);
                OSReport("Bot pixels out.............:   %8d\n", botPixOut);
                OSReport("Clr pixels in..............:   %8d\n", clrPixIn);
                OSReport("Copy clocks................:   %8d\n", copyClks);
            }
            else if (Stat[i].stat_type == VC_PERF)
            {
                OSReport("Vcache checks..............:   %8d\n", vcCheck);
                OSReport("Vcache misses..............:   %8d\n", vcMiss);
                OSReport("Vcache stalls..............:   %8d\n", vcStall);
            }
            else if (Stat[i].stat_type == MEM_PERF)
            {
                OSReport("CP requests................:   %8d\n", cpReq);
                OSReport("TC requests................:   %8d\n", tcReq);
                OSReport("CPU Rd requests............:   %8d\n", cpuRdReq);
                OSReport("CPU Wr requests............:   %8d\n", cpuWrReq);
                OSReport("DSP requests...............:   %8d\n", dspReq);
                OSReport("IO requests................:   %8d\n", ioReq);
                OSReport("VI requests................:   %8d\n", viReq);
                OSReport("PE requests................:   %8d\n", peReq);
                OSReport("RF requests................:   %8d\n", rfReq);
                OSReport("FI requests................:   %8d\n", fiReq);
            }
            else
                OSReport("%s:   %8d\n", Stat[i].text, Stat[i].cnt);
        }


        if (testData[Test].fill)
        {
            OSDumpStopwatch(&myCpuTime);
            OSDumpStopwatch(&myGpTime);
        }
        OSDumpStopwatch(&myTotTime);
}


/*---------------------------------------------------------------------------*
    Name:			PrintIntro
    
    Description:	Print usage for test
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static
void PrintIntro( void )
{
    OSReport("\n\n********************************\n");
    OSReport(" Check immediate mode performance\n");
    OSReport(" Check transform performance\n");
    OSReport("\n");
    OSReport(" This test outputs only text info\n");
    OSReport("************************************\n");
}

/*---------------------------------------------------------------------------*
    Name:			checkForOverflow
    
    Description:	return overflow bit of newFifo
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static u32
checkForOverflow( void )
{
    GXBool overhi, underlow;
    GXBool cpu_write, gp_read, fifowrap;
    u32    fifo_cnt;

    GXGetFifoStatus(&newFifo, &overhi, &underlow, &fifo_cnt, &cpu_write,
                    &gp_read, &fifowrap);

    return ((u32)fifowrap);
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
        GXNormal1x8(0);
        GXColor1x8(0);
        GXTexCoord1x8(0); //
		GXPosition1x16(5);
        GXNormal1x8(0);
        GXColor1x8(0);
        GXTexCoord1x8(1); // 
		GXPosition1x16(6);
        GXNormal1x8(0);
        GXColor1x8(0);
        GXTexCoord1x8(2); // 
		GXPosition1x16(7);			
        GXNormal1x8(0);
        GXColor1x8(0);
        GXTexCoord1x8(3); // 

		GXPosition1x16(2);
        GXNormal1x8(0);
        GXColor1x8(1);
        GXTexCoord1x8(0); //
		GXPosition1x16(6);
        GXNormal1x8(0);
        GXColor1x8(1);
        GXTexCoord1x8(1); //
		GXPosition1x16(5);
        GXNormal1x8(0);
        GXColor1x8(1);
        GXTexCoord1x8(2); //
		GXPosition1x16(3);
        GXNormal1x8(0);
        GXColor1x8(1);
        GXTexCoord1x8(3); //
	
		GXPosition1x16(1);
        GXNormal1x8(0);
        GXColor1x8(2);
        GXTexCoord1x8(0); //
		GXPosition1x16(0);
        GXNormal1x8(0);
        GXColor1x8(2);
        GXTexCoord1x8(1); //
		GXPosition1x16(4);
        GXNormal1x8(0);
        GXColor1x8(2);
        GXTexCoord1x8(2); //
		GXPosition1x16(7);
        GXNormal1x8(0);
        GXColor1x8(2);
        GXTexCoord1x8(3); //
   
 		GXPosition1x16(0);
        GXNormal1x8(0);
        GXColor1x8(3);
        GXTexCoord1x8(0); //
		GXPosition1x16(1);
        GXNormal1x8(0);
        GXColor1x8(3);
        GXTexCoord1x8(1); //
		GXPosition1x16(2);
        GXNormal1x8(0);
        GXColor1x8(3);
        GXTexCoord1x8(2); //
		GXPosition1x16(3);
        GXNormal1x8(0);
        GXColor1x8(3);
        GXTexCoord1x8(3); //
      
        GXPosition1x16(5);
        GXNormal1x8(0);
        GXColor1x8(4);
        GXTexCoord1x8(0); //
        GXPosition1x16(4);
        GXNormal1x8(0);
        GXColor1x8(4);
        GXTexCoord1x8(1); //
        GXPosition1x16(0);
        GXNormal1x8(0);
        GXColor1x8(4);
        GXTexCoord1x8(2); //
        GXPosition1x16(3);
        GXNormal1x8(0);
        GXColor1x8(4);
        GXTexCoord1x8(3); //
	 		
	 	GXPosition1x16(6);
        GXNormal1x8(0);
        GXColor1x8(5);
        GXTexCoord1x8(0); //
	 	GXPosition1x16(2);
        GXNormal1x8(0);
        GXColor1x8(5);
        GXTexCoord1x8(1); //
	 	GXPosition1x16(1);
        GXNormal1x8(0);
        GXColor1x8(5);
        GXTexCoord1x8(2); //
	 	GXPosition1x16(7);
        GXNormal1x8(0);
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
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff0000ff);// red
        GXTexCoord2f32(0.0F, 1.0F);//

		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ff00ff);// green
        GXTexCoord2f32(0.0F, 1.0F);//
	
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x0000ffff);// blue
        GXTexCoord2f32(0.0F, 1.0F);//
   
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xffff00ff);// yellow
        GXTexCoord2f32(0.0F, 1.0F);//
      
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0xff00ffff);// magenta
        GXTexCoord2f32(0.0F, 1.0F);//
	 		
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
		GXNormal3f32( SIDE, SIDE, -SIDE); //4
        GXColor1u32(0x00ffffff);// cyan
        GXTexCoord2f32(0.0F, 1.0F);//
	GXEnd();
}


/*---------------------------------------------------------------------------*
    Name:			DrawTexCubeIndx
    
    Description:	
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static
void DrawTexCubeIndx( void )
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);
		GXPosition1x16(4);
        GXTexCoord1x8(0); //
		GXPosition1x16(5);
        GXTexCoord1x8(1); // 
		GXPosition1x16(6);
        GXTexCoord1x8(2); // 
		GXPosition1x16(7);			
        GXTexCoord1x8(3); // 

		GXPosition1x16(2);
        GXTexCoord1x8(0); //
		GXPosition1x16(6);
        GXTexCoord1x8(1); //
		GXPosition1x16(5);
        GXTexCoord1x8(2); //
		GXPosition1x16(3);
        GXTexCoord1x8(3); //
	
		GXPosition1x16(1);
        GXTexCoord1x8(0); //
		GXPosition1x16(0);
        GXTexCoord1x8(1); //
		GXPosition1x16(4);
        GXTexCoord1x8(2); //
		GXPosition1x16(7);
        GXTexCoord1x8(3); //
   
 		GXPosition1x16(0);
        GXTexCoord1x8(0); //
		GXPosition1x16(1);
        GXTexCoord1x8(1); //
		GXPosition1x16(2);
        GXTexCoord1x8(2); //
		GXPosition1x16(3);
        GXTexCoord1x8(3); //
      
        GXPosition1x16(5);
        GXTexCoord1x8(0); //
        GXPosition1x16(4);
        GXTexCoord1x8(1); //
        GXPosition1x16(0);
        GXTexCoord1x8(2); //
        GXPosition1x16(3);
        GXTexCoord1x8(3); //
	 		
	 	GXPosition1x16(6);
        GXTexCoord1x8(0); //
	 	GXPosition1x16(2);
        GXTexCoord1x8(1); //
	 	GXPosition1x16(1);
        GXTexCoord1x8(2); //
	 	GXPosition1x16(7);
        GXTexCoord1x8(3); //

	GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:			DrawTexCubeDirect
    
    Description:	
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
static
void DrawTexCubeDirect( void )
{
	GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
        GXTexCoord2f32(0.0F, 1.0F);//

		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
        GXTexCoord2f32(0.0F, 1.0F);//
	
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
        GXTexCoord2f32(0.0F, 1.0F);//
   
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
        GXTexCoord2f32(0.0F, 1.0F);//
      
		GXPosition3f32(SIDE, -SIDE, -SIDE);//5
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32( SIDE, SIDE, -SIDE); //4
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, -SIDE);//0
        GXTexCoord2f32(1.0F, 1.0F);//
		GXPosition3f32(-SIDE, -SIDE, -SIDE);//3
        GXTexCoord2f32(0.0F, 1.0F);//
	 		
		GXPosition3f32(SIDE, -SIDE, SIDE); //6
        GXTexCoord2f32(0.0F, 0.0F);//
		GXPosition3f32(-SIDE, -SIDE, SIDE);//2
        GXTexCoord2f32(1.0F, 0.0F);//
		GXPosition3f32(-SIDE, SIDE, SIDE); //1
        GXTexCoord2f32(1.0F, 1.0F);//
	 	GXPosition3f32(SIDE, SIDE, SIDE);  //7
        GXTexCoord2f32(0.0F, 1.0F);//
	GXEnd();
}
