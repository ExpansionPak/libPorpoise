/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-init-gc.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-init-gc.c $
    
    2     10/19/02 6:53p Hirose
    Changed location of data file.
    
    1     9/25/01 6:23p Carl
    Sources for GD init demo.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

#include <dolphin/gd.h>

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( void );
static void DrawInit        ( void );
static void DrawTick        ( void );

static void AnimTick        ( void );

static void ParameterInit   ( void );

#ifdef LOAD_DL_FROM_FILE
static void LoadDLs   ( void );
#else
extern void CreateDLs( void );
#endif

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Display lists *************************************************************

// These are only created during runtime if LOAD_DL_FROM_FILE is not defined.
// Otherwise, these are loaded from a file.

// This DL is used with the "Draw" display list.
// It initializes state that will be used by the Draw DL.

GDLObj InitDLO;

// This DL draws a colored cube.

GDLObj DrawDLO;

// This array indicates the offsets to patch memory addresses for the
// primitive data arrays (positions and colors) referred to in the Init DL.

u32 *setArrayOffsets;

/*---------------------------------------------------------------------------*/

// Actual primitive data follows

#define SIDE 30

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

GXColor IntColor[] ATTRIBUTE_ALIGN(32) =
{
    { 255, 0, 0, 255 },   // red
    { 0, 255, 0, 255 },   // green
    { 0, 0, 255, 255 },   // blue
    { 0, 255, 255, 255 }, // cyan
    { 255, 0, 255, 255 }, // magenta
    { 255, 255, 0, 255 }, // yellow
};

// Misc data...

Mtx v;          // view matrix
u32 rot;        // current cube rotation
Mtx mv;         // model view matrix

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

    err = GDReadDLFile("gddemo/gdInit.gdl", &numDLs, &numPLs,
                       &DLDescArray, &PLDescArray);
    
    OSReport("(%d) Read %d DLs, %d PLs\n", err, numDLs, numPLs);

    ASSERTMSG(!err, "Error reading GDL file.\n");

    ASSERTMSG(numDLs == 2 && numPLs == 1, "Incorrect data in GDL file.\n");

    // Note: We put the DL length into the "offset" field, since that's
    // where the run-time code expects it.  We do this since the CreateDLs
    // function uses an oversize "length" field, and the actual valid data
    // length is saved in the "offset" field in that case.  Thus we do the
    // same thing here for consistency.

    GDInitGDLObj( &InitDLO, DLDescArray[0].ptr, DLDescArray[0].byteLength );
    GDSetCurrent( &InitDLO );
    GDSetCurrOffset( DLDescArray[0].byteLength );

    GDInitGDLObj( &DrawDLO, DLDescArray[1].ptr, DLDescArray[1].byteLength );
    GDSetCurrent( &DrawDLO );
    GDSetCurrOffset( DLDescArray[1].byteLength );

    GDSetCurrent( NULL );

    ASSERTMSG(PLDescArray[0].byteLength == 2 * sizeof(u32),
              "Incorrect patch list in GDL file.\n");

    setArrayOffsets = (u32 *) PLDescArray[0].ptr;
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
    u32          i;
    static void *basePtrs[2] = { FloatVert, IntColor };
    void        *cp;
    u32          length;

    // Load or create all the display lists and patch lists.

#ifdef LOAD_DL_FROM_FILE
    LoadDLs();
#else
    CreateDLs();
#endif

    // Now, we need to patch in the vertex array base pointers
    GDSetCurrent(&InitDLO);
    length = GDGetCurrOffset(); // preserve the offset!
    for(i=0; i<2; i++)
    {
        GDSetCurrOffset(setArrayOffsets[i]);
        cp = GDGetCurrPointer();
        GDPatchArrayPtr( basePtrs[i] );
        DCStoreRange(cp, CP_DATA_LENGTH );
    }
    GDSetCurrOffset(length);
    GDSetCurrent(NULL);
    
    // Proceed with the usual sort of initialization...

    CameraInit();   // Initialize the camera.
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
    // Draw the cube

    // Call the init DL to set up the parameters for the draw DL.
    GXCallDisplayList(GDGetGDLObjStart(&InitDLO), GDGetGDLObjOffset(&InitDLO));

    // Set up the desired model view matrix.
    // (This was written over by the Init DL.)
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    // Call the draw DL to draw the cube.
    GXCallDisplayList(GDGetGDLObjStart(&DrawDLO), GDGetGDLObjOffset(&DrawDLO));
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
    Mtx ry, rz, tm;
    f32 tx, ty;
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
