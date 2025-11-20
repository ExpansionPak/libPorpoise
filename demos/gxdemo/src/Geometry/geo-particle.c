/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     geo-particle.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Geometry/geo-particle.c $
    
    7     7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    6     5/05/01 8:56p Hirose
    removed obsoleted flags
    
    5     7/01/00 2:12p Alligator
    added title
    
    4     5/24/00 2:52a Alligator
    added texture offset for points, usage message
    
    3     3/23/00 3:48p Hirose
    updated to use DEMOPad library
    
    2     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    8     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    7     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    6     2/01/00 7:22p Alligator
    second GX update from ArtX
    
    5     1/25/00 2:50p Carl
    Changed to standardized end of test message
    
    4     1/19/00 11:27a Carl
    More fixes for new GXInit defaults
    
    3     1/18/00 3:23p Hirose
    added GXSetNumChans() and GXSetTevOrder() call
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    9     11/15/99 4:49p Yasu
    Change datafile name
    
    8     11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    7     11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    6     99/11/03 7:31p Paul
    
    5     99/10/22 3:27p Paul

 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/

#define HEIGHT              20
#define WIDTH               12

#define WIDTH_SAMPLES       32
#define HEIGHT_SAMPLES      32
#define TOTAL_SAMPLES       (WIDTH_SAMPLES * HEIGHT_SAMPLES)

#define MAX_PARTICLES       1600
#define MAX_EMISSION_RATE   50
#define EMMISION_RATE       5.0F

#define LIFESPAN_MEAN       125.0F
#define LIFESPAN_STDDEV     25.0F
#define SIZE_MEAN           200.0F
#define SIZE_MAX            640.0F
#define SIZE_STDDEV         40.0F

#define PARTICLE_SPEED      4.0F
#define GRAVITY             -0.03F
#define EVENT_TIME          160

#define ACTIVE_LIST         MAX_PARTICLES
#define FREE_LIST           (MAX_PARTICLES + 1)

#define LINE_SEGMENT        0x0001
#define PARTICLE_HAS_SIZE   0x0002
#define ANIMATE_SIZE        0x0004
#define INVERSE_ANIMATE     0x0008
#define TEXTURE_PARTICLE    0x0010
#define TEXWIDE_PARTICLE    0x0020

/*---------------------------------------------------------------------------*
   Local Typedefs
 *---------------------------------------------------------------------------*/

// Emitter State
typedef struct
{
    Mtx     mModel;             // modelling matrix (orientation)
    Vec     vPos;               // position of emitter
    Vec     vLinVel;            // linear velocity per tick
    Vec     vAngVel;            // angular velocity per tick
    f32     rEmitted;           // counts particles emitted
    f32     rEmissionRate;      // number of particles emitted per tick
    f32     rLifespanMean;      // average lifespan of emitted particles
    f32     rLifespanStdDev;    // standard deviation of lifespan of emitted particles
    f32     rSizeMean;          // average size of emitted particles
    f32     rSizeStdDev;        // standard deviation of size of emitted particles
    u32     nType;              // type of particles emitted
    GXTexOffset offset;         // texture offset for texture particles
}
Emitter;

// Particle State
typedef struct
{
    u32     nNext;          // singularly linked list for sequential access + add/delete
    Vec     vPos;           // position
    Vec     vVel;           // velocity per tick
    s32     nLifespan;      // number of ticks of life left
    u32     nType;          // defines point/line and Lifespan->{size, color, texture} mappings
    s32     nSize;          // Size of Particle (only when PARTICLE_HAS_SIZE)
    f32     tex_s, tex_t;   // Texture coordinates
}
Particle;

typedef struct
{
    Vec vPos;
    Vec vUp;
    VecPtr vpTarget;
}
Camera;

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( void );
static void CameraUpdate    (s8 stickX, s8 stickY);
static void DrawInit        ( void );
static void DrawTick        ( void );

static void AnimTick        ( void );

static void PrintIntro          ( void );

static void SetupTransforms     ( void );
static void DrawParticles       ( void );
static void DrawEmitter         ( void );

static double BoxMuller         ( void );
static void TextureParticlesInit( void );
static void ParticleInit        ( void );
static void ParticleEmit        ( Particle *prt, Emitter *em );
static void EmitterEmit         ( Emitter *em);
static void ParticleUpdate      ( void );
static void EmitterInit         ( Emitter *em );
static void EmitterUpdate       ( Emitter *em, s8 rotX, s8 rotY );

static void SendEmitterVertex   ( u8 posIndex, u8 normalIndex, 
                                  u8 colorIndex, u8 texCoordIndex );
static void SendParticlePoint   ( Vec *vPoint, u8 colorIndex );
static void SendParticleLine    ( Vec *vPoint1, Vec *vDelta, u8 colorIndex );

static u32 rndi( void );
static void srnd( u32 x );
static double rndf( void );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

#define ULONG_MAX 4294967295
static u32 seed = 1;        // Arbitrary

Emitter emMain;             // Particle emitter
Camera  cam;

Particle ParticleData[MAX_PARTICLES + 2];
u32 nNumActiveParticles;

u8          clrRGBA[4];     // color of particle

Mtx v;

TEXPalettePtr tpl = 0;
float FloatVert[] ATTRIBUTE_ALIGN(32) =  
{
    -WIDTH, HEIGHT, -WIDTH,
    -WIDTH, HEIGHT, WIDTH,
    -WIDTH, -HEIGHT, WIDTH,
    -WIDTH, -HEIGHT, -WIDTH,
    WIDTH, HEIGHT, -WIDTH,
    WIDTH, -HEIGHT, -WIDTH,
    WIDTH, -HEIGHT, WIDTH,
    WIDTH, HEIGHT, WIDTH
};

u8  ColorRGBA8[] ATTRIBUTE_ALIGN(32) =  
{
    144,  48,   0,   0,
    160,  64,   0,  16,
    176,  80,   0,  32,
    192,  96,   0,  48,
    208, 112,   0,  64,
    224, 128,   0,  80,
    240, 144,  16,  96,
    255, 160,  32, 112,
    255, 176,  48, 128,
    255, 192,  64, 144,
    255, 208,  80, 160,
    255, 224,  96, 176,
    255, 240, 128, 192,
    255, 255, 160, 208,
    255, 255, 192, 224,
    255, 255, 224, 240,
    255, 255, 255, 255, 
    240, 255, 255, 255, 
    224, 255, 255, 255, 
    208, 255, 255, 255, 
    192, 240, 255, 255, 
    176, 224, 255, 255, 
    160, 208, 255, 255, 
    144, 192, 255, 255, 
};  //GX_RGBA8

u8 EmitterColorRGBA8[] ATTRIBUTE_ALIGN(32) =
{
      0,   0,   0, 255,
     72,  72, 255, 255,
    140,  60, 140, 255,
    80,  128,   0, 255,
      0, 140, 160, 255,
    255, 255, 255, 255,
};  //GX_RGBA8

f32 FloatTex[] ATTRIBUTE_ALIGN(32) =  
{   0.0F, 0.0F, 
    1.0F, 0.0F,
    1.0F, 1.0F, 
    0.0F, 1.0F,
    0.5F, 0.5F
};

f32 FloatNorm[] ATTRIBUTE_ALIGN(32) =
{
    -1.0F, 0.0F, 0.0F,
    1.0F, 0.0F, 0.0F,
    0.0F, -1.0F, 0.0F,
    0.0F, 1.0F, 0.0F,
    0.0F, 0.0F, -1.0F,
    0.0F, 0.0F, 1.0F
};

u8 MyPointTexture[4*8] ATTRIBUTE_ALIGN(32) =
{
      0xaf, 0x0f, 0xf0, 0xf0,
      0xff, 0x0f, 0xf0, 0xff,
      0x00, 0x0f, 0xf0, 0x00,
      0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff,
      0x00, 0x0f, 0xf0, 0x00,
      0xff, 0x0f, 0xf0, 0xff,
      0x0f, 0x0f, 0xf0, 0xf0
};  //GX_TF_I4


u32 CurrentControl = 0;
u32 TypeControl = 0;
u32 SizeAnimationControl = 0;
u32 NormalControl = 0;
u32 TexCoordControl = 0;

u8  PositionShift = 0;
u8  NormalShift = 0;
u8  TexCoordShift = 0;
s32 FrameNumber;

s16 GridOrder[TOTAL_SAMPLES];

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);

    ParticleInit();
    EmitterInit( &emMain );
    DrawInit();         // Define my vertex formats and set array pointers.
    PrintIntro();

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
    Name:           TextureParticlesInit

    Description:    Initialize parameters for texture particles

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void TextureParticlesInit()
{
    u32 nI, nJ, nK;
    s16 nT;

    nJ=1;
    for(nI=0; nI<TOTAL_SAMPLES; nI++)
    {
        //GridOrder[nI] = (s16)nI;
        nJ = ((nJ*17)+19) % TOTAL_SAMPLES;
        GridOrder[nI] = (s16)nJ;
    }

    // Make a number of shuffles
    for(nI=0; nI<511; nI++)
    {
        nJ = rndi() % TOTAL_SAMPLES;
        nK = rndi() % TOTAL_SAMPLES;

        nT = GridOrder[nJ];
        GridOrder[nJ] = GridOrder[nK];
        GridOrder[nK] = nT;
    }
}


#define LINE_SEGMENT        0x0001
#define PARTICLE_HAS_SIZE   0x0002
#define ANIMATE_SIZE        0x0004
#define INVERSE_ANIMATE     0x0008
/*---------------------------------------------------------------------------*
    Name:           CameraInit
    
    Description:    Initialize the projection matrix and load into hardware.

    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit      ( void )
{
    Mtx44 p;
    Vec camPt = {0.0F, 0.0F, 650.0F};
    Vec up = {0.0F, 1.0F, 0.0F};
    
    MTXFrustum(p, 240, -240, -320, 320, 500, 2000);

    GXSetProjection(p, GX_PERSPECTIVE);

    cam.vPos = camPt;
    cam.vUp = up;
    cam.vpTarget = &emMain.vPos;

    MTXIdentity(v);
}

/*---------------------------------------------------------------------------*
    Name:           CameraUpdate

    Description:    Updates the camera object based on the joystick's state.

    Arguments:      s8 stickX, stickY    joystick direction (-127..+127, -127..+127)

    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraUpdate(s8 stickX, s8 stickY)
{
    Vec vVertical;
    Vec vHorizontal;
    Mtx m;

    vVertical.x = v[0][1];
    vVertical.y = v[1][1];
    vVertical.z = v[2][1];

    vHorizontal.x = v[0][0];
    vHorizontal.y = v[1][0];
    vHorizontal.z = v[2][0];

    MTXRotAxisDeg(m, &vHorizontal, (float)stickY * 0.01F);
    MTXMultVec(m, &cam.vPos, &cam.vPos);
    MTXRotAxisDeg(m, &vVertical, (float)stickX * 0.01F);
    MTXMultVec(m, &cam.vPos, &cam.vPos);
    MTXLookAt(v, &cam.vPos, &vVertical, cam.vpTarget);
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

    GXTexObj to;

    TEXGetPalette(&tpl, "gxTests/geo-00.tpl");

    TEXGetGXTexObjFromPalette(tpl, &to, 0);

    GXLoadTexObj(&to, GX_TEXMAP0);

    // init my point texture
    GXInitTexObj(
            &to,
            MyPointTexture,
            8, 8, // wd, ht
            GX_TF_I4,
            GX_CLAMP, GX_CLAMP,
            GX_FALSE);

    GXLoadTexObj(&to, GX_TEXMAP1);

    CameraInit();   // Initialize the camera.

    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
        GX_LO_CLEAR);
    GXSetZMode(FALSE, GX_ALWAYS, FALSE);
}

/*---------------------------------------------------------------------------*
    Name:           Options

    Description:    Selects test options

    Arguments:      u16 buttons     button status

    Returns:        none
 *---------------------------------------------------------------------------*/
static void Options ( u16 buttons )
{
    if (buttons & PAD_BUTTON_X)
    {
        CurrentControl = (CurrentControl + 1) % 3;

        switch(CurrentControl)
        {
            case 0:
                OSReport("\nParticle Type Control\n");
                break;
            case 1:
                OSReport("\nPoint Size Animation Control\n");
                break;
        }
    }

    if (CurrentControl == 0)
    {
        if(buttons & PAD_BUTTON_B)
        {
            TypeControl = (TypeControl + 1) % 13;

            switch(TypeControl)
            {
                case 0:
                    OSReport("Particle Type - Points\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType &= ~TEXTURE_PARTICLE;
                    break;
                case 1:
                    OSReport("Particle Type - Lines\n");
                    emMain.nType |= LINE_SEGMENT;
                    emMain.nType &= ~TEXTURE_PARTICLE;
                    break;
                case 2:
                    OSReport("Particle Type - Texture Points GX_TO_ZERO\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_ZERO;
                    FrameNumber = -16;
                    break;
                case 3:
                    OSReport("Particle Type - Texture Points GX_TO_SIXTEENTH\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_SIXTEENTH;
                    FrameNumber = -16;
                    break;
                case 4:
                    OSReport("Particle Type - Texture Points GX_TO_EIGHTH\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_EIGHTH;
                    FrameNumber = -16;
                    break;
                case 5:
                    OSReport("Particle Type - Texture Points GX_TO_FOURTH\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_FOURTH;
                    FrameNumber = -16;
                    break;
                case 6:
                    OSReport("Particle Type - Texture Points GX_TO_HALF\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_HALF;
                    FrameNumber = -16;
                    break;
                case 7:
                    OSReport("Particle Type - Texture Points GX_TO_ONE\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_ONE;
                    FrameNumber = -16;
                    break;
                case 8: // bubble texture
                    OSReport("Particle Type - Bubble Points GX_TO_ONE\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_ONE;
                    FrameNumber = -16;
                    break;
                case 9: // bubble texture
                    OSReport("Particle Type - Bubble Points GX_TO_HALF\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_HALF;
                    FrameNumber = -16;
                    break;
                case 10: // bubble texture
                    OSReport("Particle Type - Bubble Points GX_TO_FOURTH\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_FOURTH;
                    FrameNumber = -16;
                    break;
                case 11: // bubble texture
                    OSReport("Particle Type - Bubble Points GX_TO_EIGHTH\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_EIGHTH;
                    FrameNumber = -16;
                    break;
                case 12: // bubble texture
                    OSReport("Particle Type - Bubble Points GX_TO_SIXTEENTH\n");
                    emMain.nType &= ~LINE_SEGMENT;
                    emMain.nType |= TEXTURE_PARTICLE;
                    emMain.offset = GX_TO_SIXTEENTH;
                    FrameNumber = -16;
                    break;
            }
        }
    }
    else if (CurrentControl == 1)
    {
        if (buttons & PAD_BUTTON_B)
        {
            SizeAnimationControl = (SizeAnimationControl + 1) % 3;

            switch (SizeAnimationControl)
            {
                case 0:
                    OSReport("Point Size - Constant per particle\n");
                    emMain.nType &= ~ANIMATE_SIZE;
                    break;
                case 1:
                    OSReport("Point Size - Animated  per particle - Grow smaller\n");
                    emMain.nType |= ANIMATE_SIZE;
                    emMain.nType &= ~INVERSE_ANIMATE;
                    break;
                case 2:
                    OSReport("Point Size - Animated  per particle - Grow larger\n");
                    emMain.nType |= ANIMATE_SIZE;
                    emMain.nType |= INVERSE_ANIMATE;
                    break;
            }
        }
    }
   
    
    if (buttons & PAD_TRIGGER_L) {
        emMain.rSizeMean -= 10.0F;
    }
    if (buttons & PAD_TRIGGER_R) {
        emMain.rSizeMean += 10.0F;
    }

    if (emMain.rSizeMean < SIZE_MEAN)
        emMain.rSizeMean = SIZE_MEAN;
    if (emMain.rSizeMean > SIZE_MAX)
        emMain.rSizeMean = SIZE_MAX;

    if (buttons & PAD_TRIGGER_L) {
        OSReport("average size is %5.2f\n", emMain.rSizeMean*0.10F);
    }
    if (buttons & PAD_TRIGGER_R) {
        OSReport("average size is %5.2f\n", emMain.rSizeMean*0.10F);
    }
}

static void srnd( u32 x )
{
    seed = x;
}

static u32 rndi( void )
{

    seed = (seed * 1592653589UL) + 453816691UL;
    return seed;
}

static double rndf( void )  // 0 <= rndf() < 1
{
    return (1.0 / (ULONG_MAX + 1.0)) * rndi();
}

/*---------------------------------------------------------------------------*
    Name:           Box Muller

    Description:    Generates normal random variates
                    via the Box-Muller transform

    Arguments:      none

    Returns:        normal variate with mean 0 and standard deviation 1
 *---------------------------------------------------------------------------*/
static double BoxMuller( void )
{
    double x1, x2, w, y1;
    static double y2;
    static int use_last = 0;

    if (use_last)                   // use value from previous call
    {
        y1 = y2;
        use_last = 0;
    }
    else
    {
        do
        {
            x1 = 2.0 * rndf() - 1.0;
            x2 = 2.0 * rndf() - 1.0;
            w = (x1 * x1) + (x2 * x2);
        } while ( w > 1.0 );

        w = sqrt( (-2.0 * log( w ) ) / w );
        y1 = x1 * w;
        y2 = x2 * w;
        use_last = 1;
    }
    return y1;
}

/*---------------------------------------------------------------------------*
    Name:           ParticleInit

    Description:    Initializes the particle list pointers

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void ParticleInit( void )
{
    u32 nI;

    for(nI=1; nI<(MAX_PARTICLES+2); nI++)
    {
        ParticleData[nI-1].nNext = nI;
    }
    ParticleData[nI-1].nNext = 0;

    nNumActiveParticles = 0;

    TextureParticlesInit();
}

/*---------------------------------------------------------------------------*
    Name:           TextureParticleEmit

    Description:    Emits a texture particle, fixing its initial velocity so
                    that it hits the target spot at a specified time

    Arguments:      Particle *prt   Pointer to the particle to be emitted
                    Emitter *em     Pointer to the Emitter
                    s16 nOrder

    Returns:        none
 *---------------------------------------------------------------------------*/
static void TextureParticleEmit(Particle *prt, Emitter *em, s16 nOrder)
{
    s32 nX = ((nOrder >> 4) & 0x3e) - 31;
    s32 nY = ((nOrder << 1) & 0x3e) - 31;
    s32 nT;
    f32 rRT;     // Reciprocal of target time
    Vec vDest;

    prt->nLifespan = 200;

    vDest.x = nX * 5.0F;
    vDest.z = nY * 5.0F;
    vDest.y = 200.0F;

    MTXMultVec(em->mModel, &vDest, &vDest);

    nT = EVENT_TIME - FrameNumber;
    rRT = 1.0F / nT;

    prt->vVel.x = vDest.x * rRT;
    prt->vVel.y = (vDest.y * rRT) - (GRAVITY * 0.5F * (nT - 1));
    prt->vVel.z = vDest.z * rRT;

    prt->vPos.x = 0.0F;
    prt->vPos.y = 0.0F;
    prt->vPos.z = 0.0F;

    prt->tex_s = (nX * 0.015625F) + 0.5F;
    prt->tex_t = (nY * -0.015625F) + 0.5F;

    prt->nType = em->nType;

    if (em->nType & PARTICLE_HAS_SIZE)
    {
        prt->nSize = (s32)((float)em->rSizeMean +
        ((float)BoxMuller() * em->rSizeStdDev));
    }
}

/*---------------------------------------------------------------------------*
    Name:           ParticleEmit


    Description:    Emits a single particle, setting up its initial parameters

    Arguments:      Particle *prt   Pointer to the particle to be emitted
                    Emitter *em     Pointer to the Emitter

    Returns:        none
 *---------------------------------------------------------------------------*/
static void ParticleEmit(Particle *prt, Emitter *em)
{
    float rW;

    // Normally distributed lifespans
    prt->nLifespan = (s32)((float)em->rLifespanMean +
        ((float)BoxMuller() * em->rLifespanStdDev));

    do
    {
        prt->vVel.x = (float)(2 * rndf())-1;
        prt->vVel.z = (float)(2 * rndf())-1;
        rW = (prt->vVel.x * prt->vVel.x) +
             (prt->vVel.z * prt->vVel.z);
    }
    while (rW > 1.0F);

    prt->vVel.x *= 0.5;
    prt->vVel.z *= 0.5;
    rW *= 0.25;

    prt->vVel.y = (float) sqrt( 1.0F - rW );

    VECScale(&prt->vVel, &prt->vVel, PARTICLE_SPEED);
    MTXMultVec(em->mModel, &prt->vVel, &prt->vVel);

    VECScale(&prt->vVel, &prt->vPos, 2.5F);
    VECAdd(&em->vPos, &prt->vPos, &prt->vPos);

    prt->nType = em->nType;

    if (em->nType & PARTICLE_HAS_SIZE)
    {
        prt->nSize = (s32)((float)em->rSizeMean +
        ((float)BoxMuller() * em->rSizeStdDev));
    }
}

/*---------------------------------------------------------------------------*
    Name:           EmitterEmit

    Description:    Emits one timestep's worth of particles

    Arguments:      Emitter *em     Pointer to the emitter

    Returns:        none
 *---------------------------------------------------------------------------*/
static void EmitterEmit(Emitter *em)
{
    u32 nPrt, nI, nN, nTmp;

    nN = (u32)em->rEmitted;
    em->rEmitted += em->rEmissionRate;
    nN = (u32)em->rEmitted - nN;


    if (em->nType & TEXTURE_PARTICLE)
    {
        FrameNumber++;

        if (FrameNumber > 250)
        {
            FrameNumber = 0;
        }
        else if ((FrameNumber < 0) || (FrameNumber > 128))
        {
            return;
        }
        nN = 8;
    }

    nNumActiveParticles += nN;

    if (nNumActiveParticles > MAX_PARTICLES)
    {
        nN -= (nNumActiveParticles - MAX_PARTICLES);
        nNumActiveParticles = MAX_PARTICLES;
        OSReport("Particles Maxed Out!\n");
    }

    if (nN == 0)
    {
        return;
    }

    nPrt = FREE_LIST;

    if (em->nType & TEXTURE_PARTICLE)
    {
        for(nI=0; nI<nN; nI++)
        {
            nPrt = ParticleData[nPrt].nNext;
            TextureParticleEmit(&ParticleData[nPrt], em,
                GridOrder[(FrameNumber<<3) + nI]);
        }
    }
    else
    {
        for(nI=0; nI<nN; nI++)
        {
            nPrt = ParticleData[nPrt].nNext;
            ParticleEmit(&ParticleData[nPrt], em);
        }
    }

    nTmp = ParticleData[ACTIVE_LIST].nNext;
    ParticleData[ACTIVE_LIST].nNext = ParticleData[FREE_LIST].nNext;
    ParticleData[FREE_LIST].nNext = ParticleData[nPrt].nNext;
    ParticleData[nPrt].nNext = nTmp;
}

/*---------------------------------------------------------------------------*
    Name:           ParticleUpdate

    Description:    Updates the positions of all particles

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void ParticleUpdate( void )
{
    /*  The particles are stored in a list
     *  Sequentially step through the list.
     *  Decrement lifesepan by 1 and if it becomes <0, remove the particle.
     *  Update the position of remaining particles by adding velocity to position
     *  Update the velocity of particles due to any force fields such as gravity
     */

    u32 nPrt, *pPrev;
    Particle *prt;

    pPrev = &ParticleData[ACTIVE_LIST].nNext;
    nPrt = *pPrev;
    for(; nPrt!=FREE_LIST; nPrt=*pPrev)
    {
        prt = &ParticleData[nPrt];
        prt->nLifespan --;

        if (prt->nLifespan < 0)
        {
            // Remove particle from active list and add to head of free list

            *pPrev = prt->nNext;
            prt->nNext = ParticleData[FREE_LIST].nNext;
            ParticleData[FREE_LIST].nNext = nPrt;

            nNumActiveParticles--;
        }
        else
        {
            // Update particle

            VECAdd(&prt->vPos, &prt->vVel, &prt->vPos);
            prt->vVel.y += GRAVITY;
            pPrev = &prt->nNext;
        }
    }
}

/*---------------------------------------------------------------------------*/
static inline void SendParticlePoint( Vec *vPoint, u8 colorIndex )
{
    GXPosition3f32(vPoint->x, vPoint->y, vPoint->z);
    GXColor1x8(colorIndex);
    GXTexCoord2f32( 0.5F, 0.5F ); //texCoordIndex);
}

/*---------------------------------------------------------------------------*/
static inline void SendParticleLine( Vec *vPoint1, Vec *vDelta, u8 colorIndex )
{
    GXPosition3f32(vPoint1->x - vDelta->x * 2,
                   vPoint1->y - vDelta->y * 2,
                   vPoint1->z - vDelta->z * 2);
    GXColor1x8(colorIndex);
    GXTexCoord2f32( 0.0F, 0.0F ); //texCoordIndex);

    GXPosition3f32(vPoint1->x + vDelta->x * 2, 
                   vPoint1->y + vDelta->y * 2,
                   vPoint1->z + vDelta->z * 2);
    GXColor1x8(colorIndex);
    GXTexCoord2f32( 0.0F, 1.0F ); //texCoordIndex);
}

/*---------------------------------------------------------------------------*/
static inline void SendTexturePoint( Vec *vPoint, f32 s, f32 t )
{
    GXPosition3f32(vPoint->x, vPoint->y, vPoint->z);
    GXColor1x8(16);
    GXTexCoord2f32(s, t);
}

/*---------------------------------------------------------------------------*
    Name:           DrawParticles

    Description:    Draws the particles

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawParticles( void )
{
    Mtx mv;
    u32 nPrt;
    Particle *prt;

    GXClearVtxDesc();

    // Set Position Params

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, PositionShift);
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetArray(GX_VA_POS, &ParticleData[0].vPos, sizeof(Particle));

    // Set Color Params

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetArray(GX_VA_CLR0, ColorRGBA8, 4);

    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0,
        GX_FALSE,   // enable channel
        GX_SRC_VTX, // amb source
        GX_SRC_VTX, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_FALSE,   // enable channel
        GX_SRC_VTX, // amb source
        GX_SRC_VTX, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);

    // Set Tex Coord Params
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);

    // 8-bit indexed floating point texture coordinates
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, TexCoordShift);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    // Setup Transforms

    MTXCopy(v, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    nPrt = ParticleData[ACTIVE_LIST].nNext;

    //GXBegin(GX_POINTS, GX_VTXFMT0, (u16)nNumActiveParticles);
    for(; nPrt!=FREE_LIST; nPrt=prt->nNext)
    {
        prt = &ParticleData[nPrt];

        if (prt->nType & TEXTURE_PARTICLE) 
        {
            GXSetNumTexGens(1); // Use one texture coord
            // undocumented function
            GXEnableTexOffsets(GX_TEXCOORD0, GX_FALSE, GX_TRUE);
            
            if (TypeControl >= 8 && TypeControl <= 12) // Bubble texture
            {
                GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0A0);
                GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
            } 
            else
            {
                GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
                GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
            }
            
            if (prt->nType & ANIMATE_SIZE)
            {
                if (prt->nType & INVERSE_ANIMATE)
                {
                    GXSetPointSize((u8)((prt->nSize + 200 - prt->nLifespan)*0.10), emMain.offset);
                }
                else
                {
                    GXSetPointSize((u8)((prt->nSize + prt->nLifespan)*0.10), emMain.offset);
                }
            }
            else
            {
                GXSetPointSize((u8)(prt->nSize*0.10), emMain.offset);
            }
            GXBegin(GX_POINTS, GX_VTXFMT0, 1);
            if (TypeControl >= 8 && TypeControl <= 12)
                SendTexturePoint( &prt->vPos, 0.0F, 0.0F );
            else
                SendTexturePoint( &prt->vPos, prt->tex_s, prt->tex_t );
            GXEnd();
            continue;
        }

        GXSetNumTexGens(0);  // Use no texture (coord)
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

        if (prt->nType & LINE_SEGMENT)
        {
            if (prt->nType & PARTICLE_HAS_SIZE)
            {
                if (prt->nType & ANIMATE_SIZE)
                {
                    if (prt->nType & INVERSE_ANIMATE)
                    {
                        GXSetLineWidth((u8)((prt->nSize + 200 - prt->nLifespan)*0.05), GX_TO_ZERO);
                    }
                    else
                    {
                        GXSetLineWidth((u8)((prt->nSize + prt->nLifespan)*0.05), GX_TO_ZERO);
                    }
                }
                else
                {
                    GXSetLineWidth((u8)(prt->nSize*0.06), GX_TO_ZERO);
                }
            }

            GXBegin(GX_LINES, GX_VTXFMT0, 2);
            SendParticleLine(&prt->vPos, &prt->vVel, (u8)(prt->nLifespan>>3));
            GXEnd();
        }
        else
        {
            if (prt->nType & PARTICLE_HAS_SIZE)
            {
                if (prt->nType & ANIMATE_SIZE)
                {
                    if (prt->nType & INVERSE_ANIMATE)
                    {
                        GXSetPointSize((u8)((prt->nSize + 200 - prt->nLifespan)*0.10), GX_TO_ZERO);
                    }
                    else
                    {
                        GXSetPointSize((u8)((prt->nSize + prt->nLifespan)*0.10), GX_TO_ZERO);
                    }
                }
                else
                {
                    GXSetPointSize((u8)(prt->nSize*0.12), GX_TO_ZERO);
                }
            }
            GXBegin(GX_POINTS, GX_VTXFMT0, 1);
            SendParticlePoint(&prt->vPos, (u8)(prt->nLifespan>>3));
            GXEnd();
        }
    }
    //GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:           EmitterInit

    Description:    Initializes an emitter

    Arguments:      Emitter *em     Pointer to the emitter

    Returns:        none
 *---------------------------------------------------------------------------*/
static void EmitterInit( Emitter *em )
{
    MTXIdentity( em->mModel );
    em->vLinVel.x = em->vLinVel.y = em->vLinVel.z = 0.0F;
    em->vAngVel = em->vLinVel;
    em->rEmitted = 0.0F;
    em->rEmissionRate   = EMMISION_RATE;
    em->rLifespanMean   = LIFESPAN_MEAN;
    em->rLifespanStdDev = LIFESPAN_STDDEV;
    em->rSizeMean       = SIZE_MEAN;
    em->rSizeStdDev     = SIZE_STDDEV;
    em->nType           = PARTICLE_HAS_SIZE;
}

/*---------------------------------------------------------------------------*
    Name:           EmitterUpdate

    Description:    Emits one timestep's worth of particles

    Arguments:      Emitter *em     Pointer to the emitter
                    s8 rotX         amount to rotate about vertical
                    s8 rotY         amount to rotate about horizontal

    Returns:        none
 *---------------------------------------------------------------------------*/
static void EmitterUpdate(Emitter *em, s8 rotX, s8 rotY)
{
    Vec vVertical;
    Vec vHorizontal;
    Mtx m;

    vVertical.x = v[0][1];
    vVertical.y = v[1][1];
    vVertical.z = v[2][1];

    vHorizontal.x = v[0][0];
    vHorizontal.y = v[1][0];
    vHorizontal.z = v[2][0];

    MTXRotAxisDeg(m, &vHorizontal, (float)rotY * 0.01F);
    MTXConcat(m, em->mModel, em->mModel);
    MTXRotAxisDeg(m, &vVertical, (float)rotX * 0.01F);
    MTXConcat(m, em->mModel, em->mModel);
}

/*---------------------------------------------------------------------------*/
static void SendEmitterVertex ( u8 posIndex, u8 normalIndex,
                                u8 colorIndex, u8 texCoordIndex )
{
    GXPosition1x8(posIndex);
    GXNormal1x8(normalIndex);
    GXColor1x8(colorIndex);
    GXTexCoord1x8(texCoordIndex);
}

/*---------------------------------------------------------------------------*
    Name:           DrawEmitter

    Description:    Draws the emitter

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawEmitter( void )
{
    Mtx mv;

    GXClearVtxDesc();

    // Set Position Params

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, PositionShift);
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetArray(GX_VA_POS, FloatVert, 12);

    // Set Normal Params

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, NormalShift);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetArray(GX_VA_NRM, FloatNorm, 12);
    /*
    GXSetChanCtrl(
        GX_COLOR0,
        GX_FALSE,   // enable channel
        GX_SRC_VTX, // amb source
        GX_SRC_VTX, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);
    */

    // Set Color Params

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
    GXSetArray(GX_VA_CLR0, EmitterColorRGBA8, 4);

    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0,
        GX_FALSE,   // enable channel
        GX_SRC_VTX, // amb source
        GX_SRC_VTX, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);
    GXSetChanCtrl(
        GX_ALPHA0,
        GX_FALSE,   // enable channel
        GX_SRC_REG, // amb source
        GX_SRC_VTX, // mat source
        GX_LIGHT0,  // light mask
        GX_DF_CLAMP,// diffuse function
        GX_AF_NONE);

    // Set Tex Coord Params
    GXSetNumTexGens( 1 );
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE); //GX_REPLACE);


    // 8-bit indexed floating point texture coordinates
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, TexCoordShift);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
    GXSetArray(GX_VA_TEX0, FloatTex, 8);

    // Setup Transforms

    MTXConcat(v, emMain.mModel, mv);
    emMain.mModel[0][3] = emMain.vPos.x;
    emMain.mModel[1][3] = emMain.vPos.y;
    emMain.mModel[2][3] = emMain.vPos.z;
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);

    // Send Geometry

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

        SendEmitterVertex(1, 0, 2, 1);
        SendEmitterVertex(2, 0, 2, 2);
        SendEmitterVertex(0, 0, 1, 0);
        SendEmitterVertex(3, 0, 1, 3);

    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

        SendEmitterVertex(5, 1, 3, 1);
        SendEmitterVertex(6, 1, 4, 2);
        SendEmitterVertex(4, 1, 3, 0);
        SendEmitterVertex(7, 1, 4, 3);

    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

        SendEmitterVertex(6, 2, 0, 1);
        SendEmitterVertex(5, 2, 0, 2);
        SendEmitterVertex(2, 2, 0, 0);
        SendEmitterVertex(3, 2, 0, 3);

    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

        SendEmitterVertex(0, 3, 5, 1);
        SendEmitterVertex(4, 3, 5, 2);
        SendEmitterVertex(1, 3, 5, 0);
        SendEmitterVertex(7, 3, 5, 3);

    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);   

        SendEmitterVertex(4, 4, 3, 1);
        SendEmitterVertex(0, 4, 1, 2);
        SendEmitterVertex(5, 4, 3, 0);
        SendEmitterVertex(3, 4, 1, 3);

    GXEnd();

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

        SendEmitterVertex(2, 5, 2, 1);
        SendEmitterVertex(1, 5, 2, 2);
        SendEmitterVertex(6, 5, 4, 0);
        SendEmitterVertex(7, 5, 4, 3);

    GXEnd();  
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick

    Description:    Updates the objects in the world by one timestep

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick ( void )
{
    u16 buttons = DEMOPadGetButton(0);
    u16 down    = DEMOPadGetButtonDown(0);
    s8 stickX = DEMOPadGetStickX(0);
    s8 stickY = DEMOPadGetStickY(0);

    EmitterUpdate(&emMain, stickX, stickY);

    if (!(buttons & PAD_BUTTON_A))
    {
        EmitterEmit(&emMain);
        ParticleUpdate();
    }

    CameraUpdate(0, 0);
    Options(down);
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
    DrawEmitter();
    DrawParticles();
}

/*---------------------------------------------------------------------------*
    Name:           PrintIntro
    
    Description:    Prints the directions on how to use this demo.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void PrintIntro( void )
{
    OSReport("\n\n****************************************************\n");
    OSReport(" geo-particle - demonstrate paritcle effects\n");
    OSReport("****************************************************\n");
    OSReport("BUTTON X:  Particle Type/Particle Animation control mode\n");
    OSReport("BUTTON B:  Select a type or animation\n");
    OSReport("BUTTON A:  Pause animation\n");
    OSReport("\n");
    OSReport("TRIGGER R: Increase average size of points\n");
    OSReport("TRIGGER L: Decrease average size of points\n");
    OSReport("\n");
    OSReport("to quit hit the menu button\n");
    OSReport("*******************************************************\n");
    OSReport("\n\n");
}


