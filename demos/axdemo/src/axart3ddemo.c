/*---------------------------------------------------------------------------*
  Project:  3D sound demo for AX
  File:     ax3ddemo.c

  Copyright 1998-2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
    
  $Log: /Dolphin/build/demos/axdemo/src/axart3ddemo.c $  
    
    3     9/05/01 4:33p Eugene
    Updated AM API. 
    
    2     8/29/01 1:52p Billyjack
    
    1     8/20/01 6:05p Billyjack
    created
    
    1     7/06/01 11:50a Billyjack
    created
  $NoKeywords: $

 *---------------------------------------------------------------------------*/

#include <demo.h>

#include <dolphin/mix.h>
#include <dolphin/axfx.h>
#include <dolphin/axart.h>
#include <dolphin/sp.h>
#include <dolphin/am.h> // ARAM manager used for AX demos         
#include "math.h"       
#include "axartdemo.h"  // symbols from sndconv


/*---------------------------------------------------------------------------*
    local type used for 3D sound object
 *---------------------------------------------------------------------------*/
typedef struct
{

    AXVPB           *voice;
    SPSoundEntry    *sound;
    AXART_SOUND     axartSound;
    AXART_3D        axart3d;
    AXART_PITCH     axartPitch;     // additional pitch cents
    AXART_VOLUME    axartVolume;    // additional attenuation

} soundObject;

// two sound objects used by demo
static soundObject  helicopter;
static soundObject  cube;

// additonal articulators for making LFO effects with cube
static AXART_PITCH_MOD  cubePitchMod[2];
static AXART_VOLUME_MOD cubeVolumeMod[2];   

static u32          soundSamples;
static SPSoundTable *soundTable;


/*---------------------------------------------------------------------------*
    Callback for AX audio frames, for this demo we need to tell AX3D to update
    sound sources then run the mixer as AX3D will apply setting to the mixer
 *---------------------------------------------------------------------------*/
static void callbackAudioFrame(void)
{
    MIXUpdateSettings();
    AXARTServiceSounds();
}


/*---------------------------------------------------------------------------*
    Some stuff for video 
 *---------------------------------------------------------------------------*/
// Constants
#define GRID_LENGTH   95
#define GRID_SEGS     19

// Local structures
typedef struct
{
    Vec        xAxis;
    Vec        yAxis;
    Vec        zAxis;
    Vec        translate;

    f32        fov;
    f32        aspect;
    f32        near;
    f32        far;

    Mtx        viewMtx;

} CameraObj;

// Forward references
void            main            ( void ); 

static void     DrawInit        ( void );
static void     DrawTick        ( void );
static void     AnimTick        ( void );

static void     CameraLoad      ( CameraObj *newCam );
static void     CameraUpdate    ( CameraObj *camera );

static void     DrawGrid        ( void );
static void     DrawUI          ( void );

// Global variables
CameraObj DefaultCamera =
{
    {1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    {0.0f,-1.0f, 0.0f},
    {0.0f, 0.0f, 2.0f},
    45.0f,
    4.0f / 3.0f,
    0.1f,
    1024.0f
};

u32              Quit = 0;
CameraObj        Camera;
GXLightObj       Light;

Vec              WorldXAxis = { 1, 0, 0 };
Vec              WorldYAxis = { 0, 1, 0 };
Vec              WorldZAxis = { 0, 0, 1 };

Vec              BallPosition = { 40, 0, 0 };
f32              BallRotation = 0.0f;
Mtx              ModelMtx;

Vec              CubePosition = { 0, 0, 0 };

f32              CubeRadius = 5.0f;

GXRenderModeObj *RenderMode;


static void* myAlloc(u32 bytes)
{
    return OSAlloc(bytes);
}


static void myFree(void *p)
{
    OSFree(p);
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void voiceDrop(void *p)
{
    // if a voice is dropped we need to get rid of references to it so it is
    // no longer serviced for that instance of soundObject...
    // we stored the pointer to the soundObject in the voice for just such an
    // occation
    soundObject *so = (soundObject*)(((AXVPB*)p)->userContext);

    if (so)
    {
        // we are using the so->voice as a flag to service the axart3D
        so->voice = NULL;

        // remove the sound from AXART
        AXARTRemoveSound(&so->axartSound);
    }
}


/*---------------------------------------------------------------------------*
    Name:           startSound
    
    Description:    start halicopter and cube sound
                    
    Arguments:      so  , pointer to static soundObject
                    st  , pointer to sound table
                    i   , index of sound to start
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void startSound(soundObject *so, SPSoundTable *st, u32 i)
{
    // allocate voice for sound object
    so->voice = AXAcquireVoice(15, &voiceDrop, (u32)so);

    if (so->voice)
    {
        // get sound entry
        so->sound = SPGetSoundEntry(st, i);

        if (so->sound)
        {
            // prepare the sound addressing
            SPPrepareSound(so->sound, so->voice, 0);

            // initialize articulators
            AXARTInitArt3D      (&so->axart3d);
            AXARTInitArtPitch   (&so->axartPitch);
            AXARTInitArtVolume  (&so->axartVolume);
            
            // add articulators to sound
            AXARTAddArticulator (&so->axartSound, (AXART_ART*)&so->axart3d);
            AXARTAddArticulator (&so->axartSound, (AXART_ART*)&so->axartPitch);
            AXARTAddArticulator (&so->axartSound, (AXART_ART*)&so->axartVolume);
            
            // add sound to sound list
            so->axartSound.axvpb         = so->voice;
            so->axartSound.sampleRate    = so->sound->sampleRate; 
            AXARTAddSound(&so->axartSound);

            // start the sound
            AXSetVoiceState(so->voice, AX_PB_STATE_RUN);
        }
        else
        {
            // free the voice if we are not going to use it
            AXFreeVoice(so->voice);
        }
    }
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void addModToCube(void)
{
    // adjust volume on cube using volume articulator alreadt installed
    cube.axartVolume.attenuation = -150 << 16;
//    cube.axartPitch.cents = -1200 << 16;

    // initialize articulators
    AXARTInitArtPitchMod(&cubePitchMod[0]);
    AXARTInitArtPitchMod(&cubePitchMod[1]);
    AXARTInitArtVolumeMod(&cubeVolumeMod[0]);
    AXARTInitArtVolumeMod(&cubeVolumeMod[1]);

    // initialize LFOs
    AXARTInitLfo(&cubePitchMod[0].lfo, AXARTSine, AXART_SINE_SAMPLES, (0.3f * AXART_SINE_SAMPLES) / 200);
    AXARTInitLfo(&cubePitchMod[1].lfo, AXARTSine, AXART_SINE_SAMPLES, (5.0f * AXART_SINE_SAMPLES) / 200);
    AXARTInitLfo(&cubeVolumeMod[0].lfo, AXARTSaw, AXART_SAW_SAMPLES, (1.0f * AXART_SAW_SAMPLES) / 200);
    AXARTInitLfo(&cubeVolumeMod[1].lfo, AXARTSine, AXART_SINE_SAMPLES, (3.0f * AXART_SINE_SAMPLES) / 200);

    // assigne pitch and attenuation for articulators
    cubePitchMod[0].cents = -500 << 16;
    cubePitchMod[1].cents = 500 << 16;
    cubeVolumeMod[0].attenuation = -30 << 16;
    cubeVolumeMod[1].attenuation = 30 << 16;

    // add articulators to sound
    AXARTAddArticulator(&cube.axartSound, (AXART_ART*)&cubePitchMod[0]);
    AXARTAddArticulator(&cube.axartSound, (AXART_ART*)&cubePitchMod[1]);
    AXARTAddArticulator(&cube.axartSound, (AXART_ART*)&cubeVolumeMod[0]);
    AXARTAddArticulator(&cube.axartSound, (AXART_ART*)&cubeVolumeMod[1]);
}


/*---------------------------------------------------------------------------*
 *---------------------------------------------------------------------------*/
static void killSound(soundObject *so)
{
    if (so->voice)
    {
        // remove the sound from AXART
        AXARTRemoveSound(&so->axartSound);

        // free tyhe voice
        AXFreeVoice(so->voice);
        
        // we are using the so->voice as a flag to service the axart3D
        so->voice = NULL;
    }
}


/*---------------------------------------------------------------------------*
    Name:           main
    
    Description:    The main application loop
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
#define MAX_ARAM_BLOCKS     2
static u32 aramMemArray[MAX_ARAM_BLOCKS];

void main (void)
{ 
    u32 aramBase;
    u32 aramSize;

        DEMOInit( NULL );
        DrawInit();

        ARInit(aramMemArray, MAX_ARAM_BLOCKS);
        ARQInit();
        AIInit(NULL);
        AXInit();                       // initialize AX
        MIXInit();                      // initialize mixer
        AXARTInit();                    // initialize AXART

        AXARTSet3DDopplerScale(20.0f);
        AXARTSet3DDistanceScale(40.0f);

        AXRegisterCallback(&callbackAudioFrame);

        // for the purpose of this demo we take all the ARAM *chuckle*
        aramSize = ARGetSize() - ARGetBaseAddress();
        aramBase = ARAlloc(aramSize);
        AMInit(aramBase, aramSize); 

        // push sound samples into ARAM
        soundSamples = AMPush("/axdemo/axart/axartdemo.spd");
    
        // load sound table into main memory
        soundTable   = AMLoadFile("/axdemo/axart/axartdemo.spt", NULL);

        // initialize the sound table
        SPInitSoundTable(soundTable, soundSamples, AMGetZeroBuffer());

        // start sounds
        startSound(&helicopter, soundTable, SND_HELICOPTER);
        startSound(&cube, soundTable, SND_CUBE);
        addModToCube();

        while( !Quit )
        {   
            // Get input and animate
            AnimTick();                
        
            DEMOBeforeRender();

            // Draw the scene
            DrawTick();             

            DEMODoneRender();
        }

        killSound(&helicopter);
        killSound(&cube);
        
        AXARTQuit();
        MIXQuit();
        AXQuit();

        OSReport( "End of demo.\n" );
}


/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initialize 
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void 
DrawInit( void )
{
    GXColor black     = {0,0,0,0};

    RenderMode = DEMOGetRenderModeObj();

    // Set background color
    GXSetCopyClear( black, GX_MAX_Z24 );
    
    // Initialize the camera
    Camera = DefaultCamera;
}


static GXColor change = { 0,  64, 128, 255 };
static BOOL delta_r = 1;
static BOOL delta_g = 1;
static BOOL delta_b = 1;


/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the current scene.  
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void 
DrawTick ( )
{
    GXColor green = { 0, 255, 0,  100 };
    GXColor red   = { 255, 0, 0, 255 };
    GXColor lightGrey = { 220, 220, 220, 255 };
    GXColor darkGrey  = {  35,  35,  35, 255 };
    Mtx     mv, scale;

    // Load up the camera
    CameraLoad( &Camera );

    // Set the blend mode for transparencies
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_GREATER, 0);
    GXSetZCompLoc(GX_FALSE);    

    // Pass register color in TEV
    GXSetTevOrder( GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
    GXSetNumTexGens( 0 );
    GXSetNumTevStages( 1 );
    GXSetNumChans( 1 );
    
    // Enable lighting
    GXSetChanCtrl( GX_COLOR0A0, GX_ENABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE );
    GXSetChanAmbColor( GX_COLOR0A0, darkGrey );

    // Initialize light w/ pre-lighting
    GXInitLightPos( &Light, 0, 0, 1024.0f );
    GXInitLightColor( &Light, lightGrey );
    GXLoadLightObjImm( &Light, GX_LIGHT0 );

    // Load in the modelview matrix for sphere
    MTXConcat( Camera.viewMtx, ModelMtx, mv );
    GXLoadPosMtxImm( mv, GX_PNMTX0 );
    GXSetCurrentMtx( GX_PNMTX0 );

    // Draw unit sphere with model matrix
    GXSetChanMatColor( GX_COLOR0A0, red );
    GXDrawSphere( 20, 20 );

    // draw cube
    MTXScale(scale, CubeRadius, CubeRadius, CubeRadius);
    MTXConcat(Camera.viewMtx, scale, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    GXSetCurrentMtx( GX_PNMTX0 );
    GXSetChanMatColor( GX_COLOR0A0, change );
    GXDrawCube();


    if (delta_r)
    {
        change.r++;
        if (255 == change.r)
        {
            delta_r = 0;
        }
    }
    else
    {
        change.r--;
        if (0 == change.r)
        {
            delta_r = 1;
        }
    }

    if (delta_g)
    {
        change.g++;
        if (255 == change.g)
        {
            delta_g = 0;
        }
    }
    else
    {
        change.g--;
        if (0 == change.g)
        {
            delta_g = 1;
        }
    }

    if (delta_b)
    {
        change.b++;
        if (255 == change.b)
        {
            delta_b = 0;
        }
    }
    else
    {
        change.b--;
        if (0 == change.b)
        {
            delta_b = 1;
        }
    }


    // Disable lighting
    GXSetChanCtrl( GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE );

    // Load in the modelview matrix for grid
    // Assume identity model matrix
    GXLoadPosMtxImm( Camera.viewMtx, GX_PNMTX0 );

    // Draw green grid (transparent so draw last )
    GXSetChanMatColor( GX_COLOR0A0, green );
    DrawGrid();

    // Draw text, do this last since it clobbers lots of GX state
    DrawUI();
}

static void toggleAxMode(void)
{
    switch (AXGetMode())
    {
    case AX_MODE_STEREO:

        AXSetMode(AX_MODE_SURROUND);

        break;

    case AX_MODE_SURROUND:

        AXSetMode(AX_MODE_STEREO);

        break;
    }
}

static int buttonDown = FALSE;
/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Animates the scene
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void 
AnimTick ( void )
{
    u16        buttons;
    u16        buttonsDown;
    s8         stickX;
    s8         stickY;
    s8         subStickX;
    s8         subStickY;
    u8         triggerL;
    u8         triggerR;
    Mtx        rot;
    static f32 digAYBtnScale;


    // Get the pad status
    DEMOPadRead();
    buttons     = DEMOPadGetButton(0);
    buttonsDown = DEMOPadGetButtonDown(0);
    stickX      = DEMOPadGetStickX(0);
    stickY      = DEMOPadGetStickY(0);
    subStickX   = DEMOPadGetSubStickX(0);
    subStickY   = DEMOPadGetSubStickY(0);
    triggerL    = DEMOPadGetTriggerL(0);
    triggerR    = DEMOPadGetTriggerR(0);


    // Quit
    if(buttonsDown & PAD_BUTTON_MENU)
    {
        Quit = 1;
    }

    // toggle AX mode
    if (buttons & PAD_BUTTON_X)
    {
        if (buttonDown == FALSE)
        {
            buttonDown = TRUE;
            toggleAxMode();
        }
    }
    else
    {
        buttonDown = FALSE;
    }

    // Reset Camera
    if(buttonsDown & PAD_TRIGGER_L)
    {
        Camera = DefaultCamera;
    }

    // Shift button
    if( buttons & PAD_TRIGGER_R )
    {
        // Roll the camera about its z axis
        if(stickX != 0)
        {
            MTXRotAxis(rot, &Camera.zAxis, -stickX * 5.0f / 128.0f);
            MTXMultVec(rot, &Camera.xAxis, &Camera.xAxis);
            MTXMultVec(rot, &Camera.yAxis, &Camera.yAxis); 
        }
    }
    else
    {
        // Zoom camera
        if( buttons & (PAD_BUTTON_Y | PAD_BUTTON_A) )
        {
            if( buttonsDown & (PAD_BUTTON_Y | PAD_BUTTON_A) )
            {
                digAYBtnScale = 0.05f;
            }

            // If Y button, reverse the direction of zoom
            if( buttons & PAD_BUTTON_Y )
                digAYBtnScale = -digAYBtnScale;

            Camera.translate.x += digAYBtnScale * Camera.zAxis.x;
            Camera.translate.y += digAYBtnScale * Camera.zAxis.y;
            Camera.translate.z += digAYBtnScale * Camera.zAxis.z;

            // Restore the scale
            if( buttons & PAD_BUTTON_Y )
                digAYBtnScale = -digAYBtnScale;

            digAYBtnScale += 0.05f;
            if( digAYBtnScale > 5.0f )
                digAYBtnScale = 5.0f;
        }

        if(stickX != 0)
        {
            // Rotate camera about world Z Axis
            MTXRotAxis(rot, &WorldZAxis, -stickX * 5.0f / 128.0f);
            MTXMultVec(rot, &Camera.xAxis, &Camera.xAxis);
            MTXMultVec(rot, &Camera.yAxis, &Camera.yAxis); 
            MTXMultVec(rot, &Camera.zAxis, &Camera.zAxis); 
        }
        if(stickY != 0)
        {
            // Rotate camera about camera x axis
            MTXRotAxis(rot, &Camera.xAxis, -stickY * 5.0f / 128.0f);
            MTXMultVec(rot, &Camera.yAxis, &Camera.yAxis); 
            MTXMultVec(rot, &Camera.zAxis, &Camera.zAxis); 
        }
    
        if( subStickX != 0 )
        {
            // Dolly the camera relative to camera x axis
            f32 translate = subStickX * 5.0f / 128.0f;
            Camera.translate.x += translate * Camera.xAxis.x;
            Camera.translate.y += translate * Camera.xAxis.y;
            Camera.translate.z += translate * Camera.xAxis.z;
        }
        if( subStickY != 0 )
        {
            // Dolly the camera relative to camera y axis
            f32 translate = subStickY * 5.0f / 128.0f;
            Camera.translate.x += translate * Camera.yAxis.x;
            Camera.translate.y += translate * Camera.yAxis.y;
            Camera.translate.z += translate * Camera.yAxis.z;
        }
    }

    if (!(buttons & PAD_TRIGGER_Z))
    {
        MTXRotDeg( rot, 'z', 1.5f );
        MTXMultVec( rot, &BallPosition, &BallPosition );
        MTXTrans( ModelMtx, BallPosition.x, BallPosition.y, BallPosition.z );
    }
    
    // Update any changes to the camera
    CameraUpdate( &Camera ); 
}


/*---------------------------------------------------------------------------*
    Name:           CameraLoad
    
    Description:    Initialize the projection matrix and load into hardware.
                    
    Arguments:      newCam - camera to switch projection to   
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void
CameraLoad( CameraObj *newCam )
{
    Mtx44 p;

    MTXPerspective( p, newCam->fov, newCam->aspect, newCam->near, newCam->far );
    GXSetProjection(p, GX_PERSPECTIVE);
}


/*---------------------------------------------------------------------------*
    Name:           CameraUpdate
    
    Description:    Updates the camera's view matrix.
                    
    Arguments:      camera - camera's view matrix to update
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void 
CameraUpdate ( CameraObj *camera )
{
    Mtx transMtx;

    VECNormalize( &camera->xAxis, &camera->xAxis );
    VECNormalize( &camera->yAxis, &camera->yAxis );
    VECNormalize( &camera->zAxis, &camera->zAxis );

    MTXRowCol(camera->viewMtx,0,0) = camera->xAxis.x;
    MTXRowCol(camera->viewMtx,0,1) = camera->xAxis.y;
    MTXRowCol(camera->viewMtx,0,2) = camera->xAxis.z;
    MTXRowCol(camera->viewMtx,0,3) = 0.0F;

    MTXRowCol(camera->viewMtx,1,0) = camera->yAxis.x;
    MTXRowCol(camera->viewMtx,1,1) = camera->yAxis.y;
    MTXRowCol(camera->viewMtx,1,2) = camera->yAxis.z;
    MTXRowCol(camera->viewMtx,1,3) = 0.0F;

    MTXRowCol(camera->viewMtx,2,0) = camera->zAxis.x;
    MTXRowCol(camera->viewMtx,2,1) = camera->zAxis.y;
    MTXRowCol(camera->viewMtx,2,2) = camera->zAxis.z;
    MTXRowCol(camera->viewMtx,2,3) = 0.0F;

    MTXTrans(transMtx, -camera->translate.x, -camera->translate.y, -camera->translate.z);
    MTXConcat(camera->viewMtx, transMtx, camera->viewMtx);

    if (helicopter.voice)
    {
        f32 oldDist, newDist, hAngle, vAngle;   
        int old;
        Vec ballPosInCamera;

        // Find position of ball in camera space
        MTXMultVec( Camera.viewMtx, &BallPosition, &ballPosInCamera );

        // Compute horizontal and vertical angle
        if (ballPosInCamera.z > 0)
        {
            if (ballPosInCamera.x > 0)
                hAngle = -atan( ballPosInCamera.x / ballPosInCamera.z ) + 3.14f;
            else
                hAngle = -atan( ballPosInCamera.x / ballPosInCamera.z ) - 3.14f;

            if (ballPosInCamera.y > 0)
                vAngle = -atan( ballPosInCamera.y / ballPosInCamera.z ) + 3.14f;
            else
                vAngle = -atan( ballPosInCamera.y / ballPosInCamera.z ) - 3.14f;
        }
        else
        {
            hAngle = -atan( ballPosInCamera.x / ballPosInCamera.z );
            vAngle = -atan( ballPosInCamera.y / ballPosInCamera.z );
        }

        oldDist = helicopter.axart3d.dist;
        newDist = sqrt(((ballPosInCamera.x * ballPosInCamera.x) +
                        (ballPosInCamera.y * ballPosInCamera.y) +
                        (ballPosInCamera.z * ballPosInCamera.z)));

        old = OSDisableInterrupts();

        helicopter.axart3d.hAngle       = hAngle;
        helicopter.axart3d.vAngle       = vAngle;
        helicopter.axart3d.dist         = newDist;
        helicopter.axart3d.closingSpeed = oldDist - newDist;
        helicopter.axart3d.update       = TRUE;

        OSRestoreInterrupts(old);
    }  

    if (cube.voice)
    {
        f32 oldDist, newDist, hAngle, vAngle;   
        int old;
        Vec cubePosInCamera;

        // Find position of cube in camera space
        MTXMultVec( Camera.viewMtx, &CubePosition, &cubePosInCamera );

        // Compute horizontal and vertical angle
        if (cubePosInCamera.z > 0)
        {
            if (cubePosInCamera.x > 0)
                hAngle = -atan( cubePosInCamera.x / cubePosInCamera.z ) + 3.14f;
            else
                hAngle = -atan( cubePosInCamera.x / cubePosInCamera.z ) - 3.14f;

            if (cubePosInCamera.y > 0)
                vAngle = -atan( cubePosInCamera.y / cubePosInCamera.z ) + 3.14f;
            else
                vAngle = -atan( cubePosInCamera.y / cubePosInCamera.z ) - 3.14f;
        }
        else
        {
            hAngle = -atan( cubePosInCamera.x / cubePosInCamera.z );
            vAngle = -atan( cubePosInCamera.y / cubePosInCamera.z );
        }

        oldDist = cube.axart3d.dist;
        newDist = sqrt(((cubePosInCamera.x * cubePosInCamera.x) +
                        (cubePosInCamera.y * cubePosInCamera.y) +
                        (cubePosInCamera.z * cubePosInCamera.z)));

        old = OSDisableInterrupts();

        cube.axart3d.hAngle       = hAngle;
        cube.axart3d.vAngle       = vAngle;
        cube.axart3d.dist         = newDist;
        cube.axart3d.closingSpeed = oldDist - newDist;
        cube.axart3d.update       = TRUE;

        OSRestoreInterrupts(old);
    }
}


/*---------------------------------------------------------------------------*
    Name:           DrawGrid
    
    Description:    Draws a square grid of length GRID_LENGTH centered about
                    the origin lying on the XY world plane.  GRID_SEGS is the 
                    number of vertices per side.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void
DrawGrid ( )
{
    u32     i, j;
    f32     position[2];
    f32     increment;

    // Draw grid
    GXClearVtxDesc();
    GXSetVtxDesc( GX_VA_POS, GX_DIRECT );
    GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0 );

    increment = ((f32)GRID_LENGTH) / GRID_SEGS;

    position[1] = -GRID_LENGTH * 0.5f;
    for( i = 0; i < GRID_SEGS; i++ )
    {
        position[0] = -GRID_LENGTH * 0.5f;
        GXBegin( GX_LINESTRIP, GX_VTXFMT0, GRID_SEGS );
        for( j = 0; j < GRID_SEGS; j++ )
        {
            GXPosition3f32( position[0], position[1], 0 );
            position[0] += increment;
        }
        position[1] += increment;
        GXEnd();
    }

    position[0] = -GRID_LENGTH * 0.5f;
    for( i = 0; i < GRID_SEGS; i++ )
    {
        position[1] = -GRID_LENGTH * 0.5f;
        GXBegin( GX_LINESTRIP, GX_VTXFMT0, GRID_SEGS );
        for( j = 0; j < GRID_SEGS; j++ )
        {
            GXPosition3f32( position[0], position[1], 0 );
            position[1] += increment;
        }
        position[0] += increment;
        GXEnd();
    }
}

/*---------------------------------------------------------------------------*
    Name:           DrawUI
    
    Description:    Draw text onto the scene
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void
DrawUI ()
{
    s16   textX = 20;
    s16   textY = 20;
    
    // Initialize GX for drawing text
    DEMOInitCaption( DM_FT_OPQ, RenderMode->fbWidth, RenderMode->xfbHeight );

    switch (AXGetMode())
    {
    case AX_MODE_STEREO:

        DEMOPrintf( textX, textY, 0, "AX mode:                         AX_MODE_STEREO");

        break;

    case AX_MODE_SURROUND:

        DEMOPrintf( textX, textY, 0, "AX mode:                         AX_MODE_SURROUND");

        break;
    }
    textY += 20;
    DEMOPrintf( textX, textY, 0, "helicopter.voice:                %.8xh", helicopter.voice);
    textY += 10;
    DEMOPrintf( textX, textY, 0, "helicopter.sound.sampleRate:     %dHz", helicopter.sound->sampleRate);
    textY += 20;
    DEMOPrintf( textX, textY, 0, "helicopter.axart3d.hAngle:       %f", helicopter.axart3d.hAngle);
    textY += 10;
    DEMOPrintf( textX, textY, 0, "helicopter.axart3d.vAngle:       %f", helicopter.axart3d.vAngle);
    textY += 10;
    DEMOPrintf( textX, textY, 0, "helicopter.axart3d.dist:         %f", helicopter.axart3d.dist);
    textY += 10;
    DEMOPrintf( textX, textY, 0, "helicopter.axart3d.closingSpeed: %f", helicopter.axart3d.closingSpeed);
}