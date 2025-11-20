/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     mgt-triple-buf.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Management/mgt-triple-buf.c $
    
    9     9/07/01 7:27p Carl
    Fixed critical section in main loop.
    
    8     5/12/01 11:33p Hirose
    removed obsolete flags
    
    7     4/11/01 4:54p John
    Moved and renamed demo:
    Framebuffer/frb-triple --> Management/mgt-triple-buf (EPPC only)
    
    6     1/09/01 4:59p Carl
    Updated comments.
    
    5     10/25/00 9:40p Hirose
    fixed flags. MAC -> MACOS
    
    4     6/07/00 5:38p Tian
    Updated OSCreateThread API
    
    3     5/31/00 4:31p Carl
    Added random offset code.
    
    2     5/30/00 3:04p Carl
    Moved work from pre retrace callback to a post retrace callback.
    
    1     5/23/00 6:57p Carl
    Triple buffering demo.  Uses only two XFB's.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*
 * Quick and dirty queue implementation.
 *---------------------------------------------------------------------------*/

typedef struct QItem_ 
{
    void* writePtr;
    void* dataPtr;
    void* copyXFB;
} QItem;

#define QUEUE_MAX 5
#define QUEUE_EMPTY QUEUE_MAX

typedef struct Queue_
{
    QItem entry[QUEUE_MAX];
    u16 top;
    u16 bot;
} Queue;

/*---------------------------------------------------------------------------*
 * Data needed for triple-buffering.
 *---------------------------------------------------------------------------*/

static Queue RenderQ;   // Queue for frames in FIFO
static Queue DoneQ;     // Queue for frames finished already

static void* myXFB1;    // Pointers to the two XFB's
static void* myXFB2;
static void* copyXFB;   // Which XFB to copy to next
static void* dispXFB;   // Which XFB is being displayed now

static GXBool BPSet  = GX_FALSE;        // Is the FIFO breakpoint set?
static GXBool BPWait = GX_FALSE;        // Is breakpt reset waiting on VBlank?
static GXBool BPGo   = GX_FALSE;        // Indicates breakpt should be released

static u16 lastVCBToken = 0;    // Last sync token the VBlank callback saw
static u16 newToken = 1;        // Value to use for new sync token.

static OSThreadQueue waitingDoneRender; // Threads waiting for frames to finish

static OSThread CUThread;               // OS data for clean-up thread
static u8       CUThreadStack[4096];    // Stack for clean-up thread

extern void*   DemoFrameBuffer1;        // Where to find XFB info
extern void*   DemoFrameBuffer2;

/*---------------------------------------------------------------------------*
   Data for drawing routine.

   The macro ATTRIBUTE_ALIGN provides a convenient way to align initialized 
   arrays.  Alignment of vertex arrays to 32B IS NOT required, but may result 
   in a slight performance improvement.
 *---------------------------------------------------------------------------*/
f32 Verts_f32[] ATTRIBUTE_ALIGN(32) = 
{
//      x, y, z       
	-1.0f, -1.0f, -1.0f,	// 0:0
	 1.0f, -1.0f, -1.0f,	// 0:1
	 1.0f,  1.0f, -1.0f,	// 0:2
	-1.0f,  1.0f, -1.0f,	// 0:3
	-1.0f, -1.0f, -1.0f,	// 1:0
	-1.0f, -1.0f,  1.0f,	// 1:1
	 1.0f, -1.0f,  1.0f,	// 1:2
	 1.0f, -1.0f, -1.0f,	// 1:3
	-1.0f, -1.0f, -1.0f,	// 2:0
	-1.0f,  1.0f, -1.0f,	// 2:1
	-1.0f,  1.0f,  1.0f,	// 2:2
	-1.0f, -1.0f,  1.0f,	// 2:3
	 1.0f,  1.0f,  1.0f,	// 3:0
	 1.0f, -1.0f,  1.0f,	// 3:1
    -1.0f, -1.0f,  1.0f,	// 3:2
	-1.0f,  1.0f,  1.0f,	// 3:3
	 1.0f,  1.0f,  1.0f,	// 4:0
	-1.0f,  1.0f,  1.0f,	// 4:1
	-1.0f,  1.0f, -1.0f,	// 4:2
	 1.0f,  1.0f, -1.0f,	// 4:3
	 1.0f,  1.0f,  1.0f,	// 5:0
	 1.0f,  1.0f, -1.0f,	// 5:1
	 1.0f, -1.0f, -1.0f,	// 5:2
	 1.0f, -1.0f,  1.0f,	// 5:3
};

u8 Colors_rgba8[] ATTRIBUTE_ALIGN(32) = 
{
  //  r,   g,   b,   a
	128, 128, 128, 255,	// 0
	128, 128, 128, 255,	// 1
	128, 128, 128, 255,	// 2
	128, 128, 128, 255,	// 3
  	255, 255, 255, 255,	// 0
	255, 255, 255, 255,	// 1
	255, 255, 255, 255,	// 2
	255, 255, 255, 255,	// 3
	  0,   0,   0, 255,	// 0
	  0,   0,   0, 255,	// 1
	  0,   0,   0, 255,	// 2
	  0,   0,   0, 255,	// 3
	128, 128, 128, 255,	// 0
	128, 128, 128, 255,	// 1
	128, 128, 128, 255,	// 2
	128, 128, 128, 255,	// 3
	255, 255, 255, 255,	// 0
	255, 255, 255, 255,	// 1
	255, 255, 255, 255,	// 2
	255, 255, 255, 255,	// 3
	  0,   0,   0, 255,	// 0
	  0,   0,   0, 255,	// 1
	  0,   0,   0, 255,	// 2
	  0,   0,   0, 255,	// 3
};

static u32 ticks = 0;	// animation time counter

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void   main            ( void );
void   CameraInit      ( Mtx v );
void   DrawInit        ( void );
void   DrawTick        ( Mtx v );
void   AnimTick        ( void );
void   PrintIntro      ( void );

/*---------------------------------------------------------------------------*/

void   BPCallback      ( void );
void   VIPreCallback   ( u32 retraceCount );
void   VIPostCallback  ( u32 retraceCount );
void*  CleanupThread   ( void* param );
void   SetNextBreakPt  ( void );

void   init_queue  (Queue *q);
void   enqueue     (Queue *q, QItem *qitm);
QItem  dequeue     (Queue *q);
QItem  queue_front (Queue *q);
GXBool queue_empty (Queue *q);
u32    queue_length(Queue *q);

/*---------------------------------------------------------------------------*
   Breakpoint Interrupt Callback
 *---------------------------------------------------------------------------*/

void BPCallback ( void )
{
    QItem qitm;
    
    qitm = queue_front(&RenderQ);

    // Check whether or not the just-finished frame can be
    // copied already or if it must wait (due to lack of a
    // free XFB).  If it must wait, set a flag for the VBlank
    // interrupt callback to take care of it.

    if (qitm.copyXFB == dispXFB) 
    {
        BPWait = GX_TRUE;
    }
    else
    {
        SetNextBreakPt();
    }
}

/*---------------------------------------------------------------------------*
   Routine to move breakpoint ahead, deal with finished frames.
 *---------------------------------------------------------------------------*/

void SetNextBreakPt ( void )
{
    QItem qitm;

    // Move entry from RenderQ to DoneQ.

    qitm = dequeue(&RenderQ);

    enqueue(&DoneQ, &qitm);

    OSWakeupThread( &waitingDoneRender );

    // Move breakpoint to next entry, if any.

    if (queue_empty(&RenderQ))
    {
        GXDisableBreakPt();
        BPSet = GX_FALSE;
    }
    else
    {
        qitm = queue_front(&RenderQ);
        GXEnableBreakPt( qitm.writePtr );
    }
}

/*---------------------------------------------------------------------------*
   VI Pre Callback (VBlank interrupt)

   The VI Pre callback should be kept minimal, since the VI registers
   must be set before too much time passes.  Additional bookkeeping is
   done in the VI Post callback.

 *---------------------------------------------------------------------------*/

void VIPreCallback ( u32 retraceCount )
{
    #pragma unused (retraceCount)
    u16 token;

    // We don't need to worry about missed tokens, since 
    // the breakpt holds up the tokens, and the logic only
    // allows one token out the gate at a time.

    token = GXReadDrawSync();

    // We actually need to use only 1 bit from the sync token.

    if (token == (u16) (lastVCBToken+1))
    {
        lastVCBToken = token;

        dispXFB = (dispXFB == myXFB1) ? myXFB2 : myXFB1;

        VISetNextFrameBuffer( dispXFB );
        VIFlush();

        BPGo = GX_TRUE;
    }
}

/*---------------------------------------------------------------------------*
   VI Post Callback (VBlank interrupt)
 *---------------------------------------------------------------------------*/

void VIPostCallback ( u32 retraceCount )
{
    #pragma unused (retraceCount)

    if (BPWait && BPGo)
    {
        SetNextBreakPt();
        BPWait = GX_FALSE;
        BPGo = GX_FALSE;
    }
}

/*---------------------------------------------------------------------------*
   Cleanup Thread
 *---------------------------------------------------------------------------*/

void* CleanupThread ( void* param )
{
    #pragma unused (param)
    QItem qitm;

    while(1) {

        OSSleepThread( &waitingDoneRender );
        
        qitm = dequeue(&DoneQ);

        // Take qitm.dataPtr and do any necessary cleanup.
        // That is, free up any data that only needed to be
        // around for the GP to read while rendering the frame.
    }
}

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    Mtx         v;   // view matrix
    PADStatus   pad[PAD_MAX_CONTROLLERS]; // game pad state

    void* tmp_read;
    void* tmp_write;
    QItem qitm;
    int   enabled;
    
    srand(1);

    DEMOInit(NULL);    // Init os, pad, gx, vi
    
    init_queue(&RenderQ);
    init_queue(&DoneQ);

    OSInitThreadQueue( &waitingDoneRender );

    // Creates a new thread. The thread is suspended by default.
    OSCreateThread(
        &CUThread,                          // ptr to the thread to init
        CleanupThread,                      // ptr to the start routine
        0,                                  // param passed to start routine
        CUThreadStack+sizeof(CUThreadStack),// initial stack address
        sizeof CUThreadStack,
        14,                                 // scheduling priority
        OS_THREAD_ATTR_DETACH);             // detached by default

    // Starts the thread
    OSResumeThread(&CUThread);

    myXFB1 = DemoFrameBuffer1;
    myXFB2 = DemoFrameBuffer2;
    dispXFB = myXFB1;
    copyXFB = myXFB2;

    (void) VISetPreRetraceCallback(VIPreCallback);
    (void) VISetPostRetraceCallback(VIPostCallback);
    (void) GXSetBreakPtCallback(BPCallback);

    // The screen won't actually unblank until the first frame has
    // been displayed (until VIFlush is called and retrace occurs).
    VISetBlack(FALSE);

    pad[0].button = 0;
    
    CameraInit(v); // Initialize the camera.  
    DrawInit();    // Define my vertex formats and set array pointers.
    PrintIntro();  // Print demo directions
         
    while(!(pad[0].button & PAD_BUTTON_MENU))
    {   
        DEMOBeforeRender();

        // We must keep latency down while still keeping the FIFO full.
        // We allow only two frames to be in the FIFO at once.

        // This is a critical section that requires no interrupts to
        // happen in between the "if" and the "sleep".  The sleep will
        // reenable interrupts, allowing one to wake up this thread.

        enabled = OSDisableInterrupts();
        if (queue_length(&RenderQ) > 1) 
        {
            OSSleepThread( &waitingDoneRender );
        }
        OSRestoreInterrupts(enabled);

        // Sample inputs
        PADRead(pad);

        // Decide what to draw
        if (!(pad[0].button & PAD_BUTTON_X))
        {
            AnimTick();
        }
        
        // Draw it
        DrawTick(v);

        // End of frame code:
        GXFlush();
        
        GXGetFifoPtrs(GXGetCPUFifo(), &tmp_read, &tmp_write);

        // Create new render queue item
        qitm.writePtr = tmp_write;
        qitm.dataPtr = NULL;        // pointer to frame-related user data
        qitm.copyXFB = copyXFB;
        
        // Technically, you can work this such that you don't
        // need the OSDisabled interrupts.  You need to rework
        // the enqueue/dequeue routines a bit, though, to make
        // them non-interfere with each other.

        enabled = OSDisableInterrupts();
        enqueue(&RenderQ, &qitm);
        OSRestoreInterrupts(enabled);

        if (BPSet == GX_FALSE) {
        
            BPSet = GX_TRUE;
            GXEnableBreakPt( tmp_write );
        }

        GXSetDrawSync( newToken );
        GXCopyDisp( copyXFB, GX_TRUE);
        GXFlush();

        newToken++;
        copyXFB = (copyXFB == myXFB1) ? myXFB2 : myXFB1;
    }

    OSHalt("End of test");
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
    Vec     up      = {0.0F, 0.0F, 1.0F};
    Vec     camLoc  = {0.25F, 4.0F, 0.5F};
    Vec     objPt   = {0.0F, 0.0F, 0.0F};
    f32     left    = 0.0375F;
    f32     top     = 0.050F;
    f32     znear   = 0.1F;
    f32     zfar    = 10.0F;
    
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
    GXColor blue = {0, 0, 255, 0};

    GXSetCopyClear(blue, 0x00ffffff);

    // Set current vertex descriptor to enable position and color0.
    // Both use 8b index to access their data arrays.
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
            
    // Position has 3 elements (x,y,z), each of type f32
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    
    // Color 0 has 4 components (r, g, b, a), each component is 8b.
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    
    // stride = 3 elements (x,y,z) each of type s16
    GXSetArray(GX_VA_POS, Verts_f32, 3*sizeof(f32));
    // stride = 4 elements (r,g,b,a) each of type u8
    GXSetArray(GX_VA_CLR0, Colors_rgba8, 4*sizeof(u8));

    // Initialize lighting, texgen, and tev parameters
    GXSetNumChans(1); // default, color = vertex color
    GXSetNumTexGens(0); // no texture in this demo
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
}

/*---------------------------------------------------------------------------*
    Name:           Vertex
    
    Description:    Create my vertex format
                    
    Arguments:      t        8-bit triangle index
		    v        8-bit vertex index
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static inline void Vertex( u8 t, u8 v )
{
    u8 qv = (u8) (4 * t + v);
    GXPosition1x8(qv);
    GXColor1x8(qv);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once.
    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( Mtx v )
{
    Mtx	m;	// Model matrix.
    Mtx	mv;	// Modelview matrix.
    u8	iQuad;	// index of quad
    u8	iVert;	// index of vertex
    u32 i,n;
    
#if 1
    // Code for testing different alignments of the breakpoint
    n=(u32) (rand() % 10);
    for(i=0; i<n; i++) 
    {
        GXCmd1u8(GX_NOP);
    }
#endif
    GXSetNumTexGens( 0 );
    GXSetNumTevStages( 1 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    
    // model has a rotation about z axis
    MTXRotDeg(m, 'z', ticks);
    MTXConcat(v, m, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
#if 0
    // Test different timing
    n=(u32) (rand() % 10 + 10);
#else
    // Test constant timing
    n=1;
#endif
    for(i=0; i<n; i++) {

	GXBegin(GX_QUADS, GX_VTXFMT0, 24);
	
	// for all triangles of octahedron, ...
	for (iQuad = 0; iQuad < 6; ++iQuad)
	{
            // for all vertices of triangle, ...
            for (iVert = 0; iVert < 4; ++iVert)
            {
                Vertex(iQuad, iVert);
            }
	}

	GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Computes next time step.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( void )
{
    ticks = (u32) (OSTicksToMilliseconds(OSGetTime()) / 10);
}

/*---------------------------------------------------------------------------*
    Name:            PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:        none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n********************************\n");
    OSReport("Press the X button to pause the animation\n");
    OSReport("To quit:\n");
    OSReport("     Hit the menu button\n");
#ifdef MACOS
    OSReport("     click on the text output window\n");
    OSReport("     select quit from the menu or hit 'command q'\n");
    OSReport("     select 'don't save'\n");
#endif
    OSReport("********************************\n");
}

/*---------------------------------------------------------------------------*
 * Quick and dirty queue implementation.
 *---------------------------------------------------------------------------*/

void init_queue(Queue *q)
{
    q->top = QUEUE_EMPTY;
}

void enqueue(Queue *q, QItem *qitm)
{
    if (q->top == QUEUE_EMPTY) 
    {
        q->top = q->bot = 0;
    }
    else
    {
        q->top = (u16) ((q->top+1) % QUEUE_MAX);
    
        if (q->top == q->bot) 
        {   // error, overflow
            OSHalt("queue overflow");
        }
    }
    
    q->entry[q->top] = *qitm;
}

QItem dequeue(Queue *q)
{
    u16 bot = q->bot;

    if (q->top == QUEUE_EMPTY)
    {   // error, underflow
        OSHalt("queue underflow");
    }
    
    if (q->bot == q->top) 
    {
        q->top = QUEUE_EMPTY;
    }
    else
    {
        q->bot = (u16) ((q->bot+1) % QUEUE_MAX);
    }

    return q->entry[bot];
}

QItem queue_front(Queue *q)
{
    if (q->top == QUEUE_EMPTY)
    {   // error, queue empty
        OSHalt("queue_top: queue empty");
    }

    return q->entry[q->bot];
}

GXBool queue_empty(Queue *q)
{
    return q->top == QUEUE_EMPTY;
}

u32 queue_length(Queue *q)
{
    if (q->top == QUEUE_EMPTY) return 0;

    if (q->top > q->bot)
        return (u32) ((s32) q->top - q->bot + 1);
    else
        return (u32) ((s32) (q->top + QUEUE_MAX) - q->bot + 1);
}
