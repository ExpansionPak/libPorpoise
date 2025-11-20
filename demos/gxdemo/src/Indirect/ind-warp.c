/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     ind-warp.c

  Copyright 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Indirect/ind-warp.c $
    
    4     5/13/01 1:45a Hirose
    removed obsolete flags
    
    3     4/09/01 7:40p Hirose
    fixed matrix type for projection API (should be Mtx44)
    
    2     10/24/00 9:41a Hirose
    added explicit NULL initialization of TEXPalletePtr
    
    1     8/04/00 2:14p Carl
    Demo for indirect warp textures.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>
#define PI 3.1415926535f

/*---------------------------------------------------------------------------*
  Defines
 *---------------------------------------------------------------------------*/

#define WIDTH_LYR1         64
#define HSHIFT_LYR1         6
#define HEIGHT_LYR1        64

#define ASPECT (10.0f/7.0f)

typedef struct
{
    f32 val, min, max, step;
    char *name;
} Param;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main          ( void );
static void ParameterInit ( void );
static void TexInit       ( void );
static void TexUpdate     ( f32 phase, f32 freq, f32 amp, f32 ang, u8 func );
static void DrawInit      ( void );
static void DrawTick      ( u8 model );
static void AnimTick      ( void );
static void PrintIntro    ( void );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Modeling parameters for the object that is drawn

Mtx objMtx;
f32 scale = 1.5f;
f32 xp = 0.0f;
f32 yp = 0.0f;
f32 zp = -5.0f;

// Parameters for the indirect texture creation function

f32 phase = 0.0f;

#define P_FUNC  0
#define P_MODEL 1
#define P_FREQ  2
#define P_AMPL  3
#define P_VANG  4
#define P_ROTA  5
#define P_LAST  5

Param parm[P_LAST+1] = {
    { 1.0f, 1.0f,  3.0f, 1.0f,  "Function"  },
    { 1.0f, 1.0f,  4.0f, 1.0f,  "Model" },
    { 5.0f, 0.1f, 30.0f, 0.05f, "Frequency" },
    { 0.5f, 0.01f, 1.0f, 0.01f, "Amplitude" },
    { 0.0f, 0.0f,  0.0f, 2.0f,  "Vector Angle" },
    { 0.0f, 0.0f,  0.0f, 0.02f, "Rotation (F3)"  }, // for function 3 only
};        

u8  select = 0;

// Texture data

static TEXPalettePtr tpl = NULL;

static GXTexObj imageTexObj;
static GXTexObj indTexObj;

static u8 tmap1[WIDTH_LYR1 * HEIGHT_LYR1 * 2] ATTRIBUTE_ALIGN(32);

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);

    PrintIntro();       // Print demo directions

    TexInit();          // Set up textures

    ParameterInit();    // Set up misc. parameters

    DEMOPadRead();      // Read the joystick for this frame

    DrawInit();         // Set up drawing state

    while(1)
    {
        DEMOPadRead();  // Read the joystick for this frame

        AnimTick();     // Do animation based on input

        // Update the indirect texture
        TexUpdate(phase,
                  parm[P_FREQ].val,
                  parm[P_AMPL].val,
                  parm[P_ROTA].val,
                  (u8) parm[P_FUNC].val);

        DEMOBeforeRender();

        DrawTick((u8) parm[P_MODEL].val);     // Draw the model.

        DEMODoneRender();
    }

    OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Name:           ParameterInit

    Description:    Initialize parameters for single frame display

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void ParameterInit( void )
{
    MTXIdentity(objMtx);
}

/*---------------------------------------------------------------------------*
    Name:           TexInit
    
    Description:    Sets up the texture state.

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void TexInit( void )
{
    // Get image texture from tpl

    TEXGetPalette(&tpl, "gxTests/tex-01.tpl");

    GXInitTexObjLOD( &imageTexObj,              // texture object
                     GX_LINEAR, GX_LINEAR,      // min/mag filter
                     0.0F, 0.0F,                // min/max LOD
                     0.0F,                      // LOD bias
                     GX_FALSE,                  // bias clamp
                     GX_FALSE,                  // do edge LOD
                     GX_ANISO_1 );              // max anisotropy

    TEXGetGXTexObjFromPalette(tpl, &imageTexObj, 3);

    // Set up indirect texture for bilinear interpolation

    // Indirect texture data will be created later.
    // Right now, we only need the pointer to where it will be.

    GXInitTexObj(&indTexObj,            // texture object
                 (void*) &tmap1,        // data
                 WIDTH_LYR1,            // width
                 HEIGHT_LYR1,           // height
                 GX_TF_IA8,             // format
                 GX_REPEAT,             // wrap s
                 GX_REPEAT,             // wrap t
                 GX_FALSE);             // mipmap

    GXInitTexObjLOD( &indTexObj,                // texture object
                     GX_LINEAR, GX_LINEAR,      // min/mag filter
                     0.0F, 0.0F,                // min/max LOD
                     0.0F,                      // LOD bias
                     GX_FALSE,                  // bias clamp
                     GX_FALSE,                  // do edge LOD
                     GX_ANISO_1 );              // max anisotropy
}

/*---------------------------------------------------------------------------*
    Name:           TexUpdate
    
    Description:    Updates the indirect texture map.

    Arguments:      phase, frequency, amplitude, rotation, function select

    Returns:        none
 *---------------------------------------------------------------------------*/
static void TexUpdate( f32 phase, f32 freq, f32 amp, f32 rot, u8 func )
{
    s32 nJ, nI, off;
    u8  nS, nT;
    f32 dx, dy, di, id;

    // Create indirect texture

    for (nJ=0; nJ<HEIGHT_LYR1; nJ++)
    {
        for (nI=0; nI<WIDTH_LYR1; nI++)
        {
            // compute the direction vector
            if (func <= 2)
            {
                dx = ((f32) nI / WIDTH_LYR1) - 0.5f;
                dy = ((f32) nJ / HEIGHT_LYR1) - 0.5f;
            } else {
                dx = (f32) nI / WIDTH_LYR1  * cosf(rot);
                dy = (f32) nJ / HEIGHT_LYR1 * sinf(rot);
            }

            // normalize the vector
            if (dx != 0.0f || dy != 0.0f)
            {
                di = sqrtf(dx*dx+dy*dy);
                id = 1.0f/di;
                dx *= id;
                dy *= id;
            } else {
                di = id = 0.0f;
            }
            
            // use inverse distance for function 2
            if (func == 2) 
            {
                di = id * 0.1f;
            }

            // compute magnitude based upon sin function of distance, phase
            di = sinf(di * freq + phase);
            
            // compute the actual indirect offsets
            nS = (u8) ((dx * di) * amp * 127.0f + 128.0f);
            nT = (u8) ((dy * di) * amp * 127.0f + 128.0f);

            // This offset calculation works for map widths that are
            // powers of 2
            off = ((nI & 3) | ((nI >> 2) << 4) | ((nJ & 3) << 2) |
                   ((nJ >> 2) << (4+(HSHIFT_LYR1-2)))) * 2;
            
            tmap1[off+0] = nS;
            tmap1[off+1] = nT;
        }
    }

    // Important! We must flush the texture data from the CPU cache
    DCFlushRange((void *) tmap1, WIDTH_LYR1 * HEIGHT_LYR1 * 2);
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Sets up the graphics state.

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{
    Mtx   idtMtx;
    Mtx44 prjMtx;

    // Misc setup
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetCullMode(GX_CULL_BACK);

    // Set up textures
    GXLoadTexObj(&imageTexObj, GX_TEXMAP0);
    GXLoadTexObj(&indTexObj, GX_TEXMAP1);

    // Set up texgen
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);

    // Set up TEV and such...
    GXSetNumTevStages(1);
    GXSetNumIndStages(1);

    // One texture coordinate, no colors.
    GXSetNumTexGens(1);
    GXSetNumChans(0);

    // Indirect Stage 0 -- Lookup indirect map
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);

    // The image map is four times bigger than the indirect map
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_4, GX_ITS_4);

    // Stage 0 -- Output texture color
    //
    // TEVPREV = TEXC/TEXA

    GXSetTevIndWarp(GX_TEVSTAGE0,       // tev stage
                    GX_INDTEXSTAGE0,    // indirect stage
                    GX_TRUE,            // signed offsets?
                    GX_FALSE,           // replace mode?
                    GX_ITM_0);          // ind matrix select

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    MTXIdentity(idtMtx);
    GXLoadPosMtxImm(idtMtx, GX_PNMTX0);
    GXLoadTexMtxImm(idtMtx, GX_TEXMTX0, GX_MTX2x4);

    MTXFrustum(prjMtx, 0.25f, -0.25f, -0.25f*ASPECT, 0.25f*ASPECT, 1.0f, 15.0f);
    GXSetProjection(prjMtx, GX_PERSPECTIVE);
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick

    Description:    Draw the current model once.  

    Arguments:      which model

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( u8 model )
{
    Mtx txMtx;

    MTXIdentity(txMtx);

    switch(model)
    {
      case 1:
        // Draw one copy of the texture per cube side
        GXLoadTexMtxImm(txMtx, GX_TEXMTX0, GX_MTX2x4);
        GXDrawCube();
        break;
      case 2:
        // Draw nine copies of the texture per cube side
        txMtx[0][0] = 3.0f;
        txMtx[1][1] = 3.0f;
        GXLoadTexMtxImm(txMtx, GX_TEXMTX0, GX_MTX2x4);
        GXDrawCube();
        break;
      case 3:
        // Draw two copies of the texture on a sphere
        txMtx[0][0] = 2.0f;
        GXLoadTexMtxImm(txMtx, GX_TEXMTX0, GX_MTX2x4);
        GXDrawSphere(12, 20);
        break;
      case 4:
        // Draw two copies of the texture on a torus
        txMtx[1][1] = 2.0f;
        GXLoadTexMtxImm(txMtx, GX_TEXMTX0, GX_MTX2x4);
        GXDrawTorus(0.3f, 12, 20);
        break;
    }
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick

    Description:    Animate the scene.  

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( void )
{
    Mtx tmpMtx;
    Mtx posMtx;
    f32 indMtx[2][3];
    f32 rAng1, rAng2, rAng3;
    u8  act = 0;
    u8  show = 0;
    static u8 first = 1;
    u16 press = DEMOPadGetButton(0);
    u16 down = DEMOPadGetButtonDown(0);

    if (first == 1)
    {
        first = 0;
        show = 1;
    }

    if (down & PAD_BUTTON_X)
    {
        select = (u8) ((select == P_LAST) ? 0 : select + 1);
        show = 1;
    }
    else if (down & PAD_BUTTON_Y)
    {
        select = (u8) ((select == 0) ? P_LAST : select - 1);
        show = 1;
    }

    if (show)
    {
        OSReport("%s select\n", parm[select].name);
    }

    if (select == P_FUNC || select == P_MODEL)
    {
        if (down & PAD_BUTTON_A)
            act = 1;
        else if (down & PAD_BUTTON_B)
            act = 2;
    } else {
        if (press & PAD_BUTTON_A)
            act = 1;
        else if (press & PAD_BUTTON_B)
            act = 2;
    }
    
    if (act == 1)
    {
        parm[select].val += parm[select].step;
        if (parm[select].max != 0.0f && parm[select].val > parm[select].max)
            parm[select].val = parm[select].max;
    }
    else if (act == 2) 
    {
        parm[select].val -= parm[select].step;
        if (parm[select].min != 0.0f && parm[select].val < parm[select].min)
            parm[select].val = parm[select].min;
    }

    if (act)
    {
        OSReport("%s = %f\n", parm[select].name, parm[select].val);
    }

#if 0
    xp += DEMOPadGetStickX(0)/1000.0f;
    yp -= DEMOPadGetStickY(0)/1000.0f;
#endif
    rAng1 = - DEMOPadGetTriggerR(0) / 100.0f
            + DEMOPadGetTriggerL(0) / 100.0f;
    rAng2  =  DEMOPadGetStickX(0) / 10.0f;
    rAng3  = -DEMOPadGetStickY(0) / 10.0f;

    scale +=  DEMOPadGetSubStickY(0)/1000.0f;
    if (scale < 0.01f)
        scale = 0.01f;
    else if (scale > 100.0f)
        scale = 100.0f;

    // Compute rotations...
    MTXRotDeg(tmpMtx, 'x', rAng3);
    MTXConcat(tmpMtx, objMtx, objMtx);
    MTXRotDeg(tmpMtx, 'y', rAng2);
    MTXConcat(tmpMtx, objMtx, objMtx);
    MTXRotDeg(tmpMtx, 'z', rAng1);
    MTXConcat(tmpMtx, objMtx, objMtx);

    // Compute the rest...
    MTXTrans(tmpMtx, xp, yp, zp);
    MTXConcat(tmpMtx, objMtx, posMtx);
    MTXScale(tmpMtx, scale, scale, scale);
    MTXConcat(posMtx, tmpMtx, posMtx);

    GXLoadPosMtxImm(posMtx, GX_PNMTX0);

    // Set indirect matrix (vector angle)

    MTXRotDeg(tmpMtx, 'z', parm[P_VANG].val);

    indMtx[0][0] = tmpMtx[0][0]*0.5f;
    indMtx[0][1] = tmpMtx[0][1]*0.5f;
    indMtx[0][2] = 0.0f;
    indMtx[1][0] = tmpMtx[1][0]*0.5f;
    indMtx[1][1] = tmpMtx[1][1]*0.5f;
    indMtx[1][2] = 0.0f;

    GXSetIndTexMtx(GX_ITM_0, indMtx, 1);

    // Phase

    if (!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {
        phase += 0.02f;
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
    OSReport("************************************************\n");
    OSReport("ind-tile-test: tiled texture map using indirect textures\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the start button\n");
    OSReport("main stick   : rotates object X/Y\n");
    OSReport("sub stick    : scales object\n");
    OSReport("l/r triggers : rotates object Z\n");
    OSReport("A/B buttons  : changes paramter value\n");
    OSReport("X/Y buttons  : selects parameter\n");
    OSReport("\n");
    OSReport("************************************************\n\n");
}
