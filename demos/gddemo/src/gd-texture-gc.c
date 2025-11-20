/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     gd-texture-gc.c

  Copyright 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gddemo/src/gd-texture-gc.c $
    
    5     10/19/02 6:53p Hirose
    Changed location of data file.
    
    4     10/11/01 4:05p Carl
    Added use of color-index texture and TLUT.
    
    3     9/25/01 6:46p Carl
    Adjusted #pragma usage.
    
    2     9/24/01 2:23p Hirose
    Changed flag definition.
    
    1     9/19/01 4:27p Carl
    Sources for GD texture demo.
    
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

void        main            ( void );

static void CameraInit      ( void );
static void DrawInit        ( void );
static void DrawTick        ( void );

static void AnimTick        ( void );

static void ParameterInit   ( void );

static void PatchTexPtrs(u32 numTplFiles, TEXPalettePtr tplFilePtrs[],
                         u32 numTextures, u32 tplFileNums[], u32 tplTexNums[], 
                         GDLObj *dlObj, u32 dlOffsets[]);

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

// This DL draws a textured cube.  It must be paired with the Init DL.

GDLObj DrawDLO;

// This array indicates the offsets to patch memory addresses for the
// primitive data arrays (positions, normals, texture coordinates)
// referred to in the Init DL.

u32 *setArrayOffsets;

// This array tells us the offsets for where to patch the memory addresses
// for the textures in the Draw DL.

u32 *texAddrOffsets;

// This array tells us the offsets for where to patch the main memory
// addresses for loading the TLUTs in the Init DL.

u32 *tlutAddrOffsets;

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
    Name:           PatchTexPtrs
    
    Description:    This routine will look up TPL texture references and will
                    patch a DL with HW pointers to the actual texture data.
                    The references are provided as a list of TPL file numbers
                    and a list of texture numbers within the given TPL's, as
                    well as a list of TPL TexPalettePtr's.

                    This function uses a GDLObj to reference the DL.  The 
                    dlOffsets point to two bytes after the GDSetTexImgPtr 
                    commands within the DL (ie, they point directly to the
                    bytes to be patched).

                    Texture Lists: (length = numTextures)

                    entry   TPL file #   TPL texture #    DL offset
                           (tplFileNums) (tplTexNums)    (dlOffsets)
                    -----   ----------   -------------   -----------
                      0         a              i              x
                      1         b              j              y
                      2         c              k              z

                    TPL List: (length = numTplFiles)

                    entry   TEXPalettePtr's
                             (tplFilePtrs)
                    -----   ---------------
                      0           A
                      1           B
                      2           C

                    You should have already called TEXGetPalette for all the
                    TPL files you are going to use.

    Arguments:      numTplFiles: input, length of tplFilePtrs list
                    tplFilePtrs: input, list of pointers to TPL file data
                    numTextures: input, number of textures to be looked up
                    tplFileNums: input, which TPL file number per texture
                    tplTexNums:  input, which TPL texture number per texture
                    dlObj:       input, should point to the DL to patch
                    dlOffsets:   input, offsets to GDSetTexImgPtr data in DL
                                        (data = command start + 2)
    Returns:        none
 *---------------------------------------------------------------------------*/

static void PatchTexPtrs(u32 numTplFiles, TEXPalettePtr tplFilePtrs[],
                         u32 numTextures, u32 tplFileNums[], u32 tplTexNums[], 
                         GDLObj *dlObj, u32 dlOffsets[])
{
#ifndef _DEBUG
#pragma unused(numTplFiles)
#endif
    u32   i;
    u32   fn;
    TEXDescriptorPtr tdescp;
    void *tp;
    void *cp;
    u32   saveOffset;
    
    GDSetCurrent(dlObj);

    // Note: we preserve the current offset just in case it is used
    // to hold something important, such as the actual data length.
    saveOffset = GDGetCurrOffset();

    for(i=0; i<numTextures; i++)
    {
        fn = tplFileNums[i];
        ASSERTMSG( fn < numTplFiles, "TPL number out of range");

        // Get the texture descriptor
        tdescp = TEXGet(tplFilePtrs[fn], tplTexNums[i]);
        // Get the data ptr
        tp = (void *) tdescp->textureHeader->data;

        // Now patch it in
        GDSetCurrOffset(dlOffsets[i]);  // set the offset
        cp = GDGetCurrPointer();        // save ptr for flushing later
        GDPatchTexImgPtr(tp);           // patch in the texture address ptr

        // We assume that the patches will lie within separate
        // 32B ranges, so go ahead and flush it to memory now.
        // If more than 1 patch is in the same range, then we're
        // just doing extra work (no harm done).
        DCStoreRange(cp, BP_DATA_LENGTH);
    }

    // Because we are flushing as we go along, we don't have
    // to flush the entire list to memory later.  If we didn't
    // flush as we went along, we'd need this:
    //
    // GDFlushCurrToMem();
    
    // Restore the offset
    GDSetCurrOffset(saveOffset);

    GDSetCurrent(NULL); // bug-prevention
}


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

    err = GDReadDLFile("gddemo/gdTextr.gdl", &numDLs, &numPLs,
                       &DLDescArray, &PLDescArray);
    
    OSReport("(%d) Read %d DLs, %d PLs\n", err, numDLs, numPLs);

    ASSERTMSG(!err, "Error reading GDL file.\n");

    ASSERTMSG(numDLs == 2 && numPLs == 3, "Incorrect data in GDL file.\n");

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

    ASSERTMSG(PLDescArray[0].byteLength == 3 * sizeof(u32),
              "Incorrect data in Patch List 0.\n");

    ASSERTMSG(PLDescArray[1].byteLength == 6 * sizeof(u32),
              "Incorrect data in Patch List 1.\n");

    ASSERTMSG(PLDescArray[2].byteLength == 1 * sizeof(u32),
              "Incorrect data in Patch List 2.\n");

    setArrayOffsets = (u32 *) PLDescArray[0].ptr;
    texAddrOffsets  = (u32 *) PLDescArray[1].ptr;
    tlutAddrOffsets = (u32 *) PLDescArray[2].ptr;
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
    u32           i;
    u32           numTpls = 1; // How many TPL files we have
    TEXPalettePtr tpls[1];     // pointer to each
    u32           numTexs = 6; // How many textures in our display list
    static u32    tplFileNums[6]={0, 0, 0, 0, 0, 0};   // which file for each
    static u32    tplTexNums[6] ={1, 2, 6, 10, 13, 3}; // which descriptor
    static void  *basePtrs[3] = { FloatVert, FloatNorm, FloatTex };
    void         *cp;
    u32           length;

    TEXDescriptorPtr tdescp;

    GXLightObj MyLight;
    GXColor White = {255, 255, 255, 255};

    // Load or create all the display lists and patch lists.

#ifdef LOAD_DL_FROM_FILE
    LoadDLs();
#else
    CreateDLs();
#endif

    tpls[0] = 0; // important: this must be initialized to 0!

    // In tex-06.tpl:
    //
    // index width height format ws wt mnf mgf mnl mxl lb mm face
    //   0    128   128   RGBA8  r  r  l/l  l   0   7   0 Y   
    //   1    128   128   RGB5A3 r  r  l/l  l   0   7   0 Y   1
    //   2    128   128   CMPR   r  r  l/l  l   0   7   0 Y   2
    //   3    128   128   CI8    r  r  l/l  l   0   7   0 N   6
    //   4    128   128   RGB565 r  r  l/l  l   0   7   0 Y  
    //   5    128   128   RGBA8  r  r  l/l  l   0   7   0 Y   
    //   6    128   128   RGBA8  r  r  l/l  l   0   7   0 Y   3
    //   7    128   128   RGB565 r  r  l/l  l   0   7   0 Y
    //   8    128   128   CMPR   r  r  l/l  l   0   7   0 Y
    //   9    128   128   RGB5A3 r  r  l/l  l   0   7   0 Y
    //  10    128   128   I4     r  r  l/l  l   0   7   0 Y   4
    //  11    128   128   IA4    r  r  l/l  l   0   7   0 Y
    //  12    128   128   I8     r  r  l/l  l   0   7   0 Y
    //  13    128   128   IA8    r  r  l/l  l   0   7   0 Y   5

    // Load all the TPL files
    TEXGetPalette(&tpls[0], "gxTests/tex-06.tpl");

    // Set up the textures in the DrawList
    //
    // We need to insert their memory addresses into the DrawList.

    // We get the texture data addresses from the TPL file(s)
    // and patch them into the GDLObj using texAddrOffsets to tell where.
    PatchTexPtrs(numTpls, tpls, numTexs, tplFileNums, tplTexNums,
                 &DrawDLO, texAddrOffsets);

    // Now, we need to patch in the vertex array base pointers
    GDSetCurrent(&InitDLO);
    length = GDGetCurrOffset(); // preserve the offset!

    for(i=0; i<3; i++)
    {
        GDSetCurrOffset( setArrayOffsets[i] );
        cp = GDGetCurrPointer();
        GDPatchArrayPtr( basePtrs[i] );
        DCStoreRange( cp, CP_DATA_LENGTH );
    }

    // Patch the address for the TLUT for texture #3 (the color-index one).
    tdescp = TEXGet( tpls[0], 3 );
    GDSetCurrOffset( tlutAddrOffsets[0] );
    cp = GDGetCurrPointer();
    GDPatchTlutPtr( tdescp->CLUTHeader->data );
    DCStoreRange( cp, BP_DATA_LENGTH );

    // Restore the original offset, which reflects the valid data length.
    GDSetCurrOffset(length);
    GDSetCurrent(NULL);

    // Set ALL texture coordinates to scale manually.
    // This way, GX won't try to do any automatic scaling.
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD3, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD4, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD5, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD6, GX_ENABLE, 1, 1);
    GXSetTexCoordScaleManually(GX_TEXCOORD7, GX_ENABLE, 1, 1);

    // Proceed with the usual sort of initialization...

    CameraInit();   // Initialize the camera.

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
    GXSetTevOp   (GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // Fix up normals
    for(i = 0; i < 24; i++)
    {
        FloatNorm[i] *= NORM;
    }

    // Flush new normals to memory
    DCFlushRange((void *)FloatNorm, sizeof(FloatNorm));
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
    Mtx ry, rz, mv, tm, mn;
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
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    MTXInverse(mv, mn);
    MTXTranspose(mn, mn);
    GXLoadNrmMtxImm(mn, GX_PNMTX0);
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
