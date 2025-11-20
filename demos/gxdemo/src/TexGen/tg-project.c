/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-project.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-project.c $
    
    6     7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    5     5/12/01 2:13a Hirose
    removed obsolete flags.
    
    4     7/07/00 5:57p Dante
    PC Compatibility
    
    3     3/24/00 6:56p Hirose
    changed to use DEMOPad library
    
    2     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    1     3/06/00 12:12p Alligator
    move from tests/gx and rename
    
    6     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     1/25/00 2:58p Carl
    Changed to standardized end of test message
    
    3     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    20    11/15/99 4:49p Yasu
    Change datafile name
    
    19    10/22/99 8:08p Yasu
    Fix Component Type of color
    
    18    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    17    9/23/99 3:07p Ryan
    Added OSHalt at the end of the demo
    
    16    9/17/99 5:26p Ryan
    added new DEMO calls
    
    15    9/15/99 1:38p Ryan
    update to fix compiler warnings
    
    14    9/08/99 11:22a Ryan
    added __SINGLEFRAME functionality
    
    13    9/01/99 5:45p Ryan
    
    12    8/28/99 12:08a Yasu
    Change enum name GX_MAT_3x4 -> GX_MTX3x4
    
    11    8/18/99 11:27a Ryan
    
    10    8/17/99 4:11p Ryan
    
    9     8/13/99 11:48a Ryan
    changed .tpl directory
    
    8     7/28/99 4:22p Ryan
    
    7     7/28/99 11:41a Ryan
    
    6     7/23/99 2:55p Ryan
    changed dolphinDemo.h to demo.h
    
    5     7/23/99 12:37p Ryan
    
    4     7/23/99 12:16p Ryan
    included dolphinDemo.h
    
    
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( Mtx v );
static void ViewInit        ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( void );
static void VertexLightInit ( void );

static void MakeModelMtx    ( Vec xAxis, Vec yAxis, Vec zAxis, Mtx m );
static void AnimTick        ( Mtx v );

static void DrawFrust       ( void );

static void TextureLightInit( Mtx rot );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
// Vectors to keep track of the camera's coordinate system orientation
Vec CamX = {1.0F, 0.0F, 0.0F};  
Vec CamY = {0.0F, 1.0F, 0.0F};
Vec CamZ = {0.0F, 0.0F, 1.0F};

Vec LightY = {0.0F, 1.0F, 0.0F};    
Vec LightX = {1.0F, 0.0F, 0.0F};
Vec LightZ = {0.0F, 0.0F, 1.0F};

Mtx v, m;

// Scale for the camera's distance from the object
float CameraLocScale = 10;

Mtx lv;

float xmin = -.5F, xmax = .5F;
float ymin = -.5F, ymax = .5F;
float nnear = 5.0F;
float ffar = 100.0F;
float distance = -45.0F;

u8  CurrentControl = 0;
u8  CurrentModel = 0;
u8  CurrentTexture = 0;
GXTevMode   CurrentTevMode = GX_DECAL;

TEXPalettePtr tpl = 0;

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();         // Define my vertex formats and set array pointers.

    VertexLightInit();

    DEMOPadRead();      // Read the joystick for this frame

    // While the quit button is not pressed
    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU)) 
    {   
        DEMOPadRead();      // Read the joystick for this frame

        // Do animation based on input
        AnimTick(v);    
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
static void CameraInit      ( Mtx v )
{
    Mtx44 p;
    
    MTXFrustum(p, .24F * CameraLocScale,-.24F * CameraLocScale,
               -.32F * CameraLocScale, .32F * CameraLocScale, 
               .5F * CameraLocScale, 20.0F * CameraLocScale);

    GXSetProjection(p, GX_PERSPECTIVE);
    
    ViewInit(v);    
}

/*---------------------------------------------------------------------------*
    Name:           ViewInit
    
    Description:    Initialize the view matrix.
                    
    Arguments:      v   view matrix
                    cameraLocScale  value used to determine camera's distance 
                    from the object
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void ViewInit ( Mtx v )
{
    Vec camPt = {0.0F, 0.0F, 8.0F};
    Vec up = {0.0F, 1.0F, 0.0F};
    Vec origin = {0.0F, 0.0F, 0.0F};
    
    camPt.x *= CameraLocScale;  // Scale camPt by cameraLocScale 
    camPt.y *= CameraLocScale; 
    camPt.z *= CameraLocScale;

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
    GXTexObj to;
    TEXDescriptorPtr tdp;

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    
    CameraInit(v);  // Re-Initialize the camera.

    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetNumChans(1);

    TEXGetPalette(&tpl, "gxTests/tg-01.tpl");
    tdp = TEXGet(tpl, 0);

    GXInitTexObj(&to, 
                 tdp->textureHeader->data, 
                 tdp->textureHeader->width, 
                 tdp->textureHeader->height, 
                 (GXTexFmt)tdp->textureHeader->format,
                 GX_CLAMP, 
                 GX_CLAMP, 
                 GX_FALSE); 
    
    GXInitTexObjLOD(&to, 
                    tdp->textureHeader->minFilter, 
                    tdp->textureHeader->magFilter, 
                    tdp->textureHeader->minLOD, 
                    tdp->textureHeader->maxLOD, 
                    tdp->textureHeader->LODBias, 
                    GX_FALSE,
                    tdp->textureHeader->edgeLODEnable,
                    GX_ANISO_1); 

    GXLoadTexObj(&to, GX_TEXMAP0);

    MTXScale(m, 20.0F, 20.0F, 20.0F);
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
    Mtx mv;
    
    GXSetTevOp(GX_TEVSTAGE0, CurrentTevMode);
    GXSetNumTexGens(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    GXSetChanCtrl(
        GX_COLOR0,
        TRUE,    // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);

    MTXConcat(v, m, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    MTXInverse(mv, mv);
    MTXTranspose(mv, mv);
    GXLoadNrmMtxImm(mv, GX_PNMTX0);
    switch(CurrentModel)
    {
        case 0: 
            GXDrawCube();
            break;
        case 1:
            GXDrawDodeca();
            break;
        case 2:
            GXDrawCylinder(20);
            break;
        case 3:
            GXDrawSphere1(3);
            break;
        case 4:
            GXDrawOctahedron();
            break;
        case 5:
            GXDrawIcosahedron();
            break;
        case 6:
            GXDrawTorus(.3F, 10, 50);
            break;
    }

    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(
        GX_COLOR0,
        FALSE,    // disable channel
        GX_SRC_VTX,  // amb source
        GX_SRC_VTX,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
    DrawFrust();
}

static void VertexLightInit ( void )
{
    GXLightObj MyLight;
    GXColor color = {255, 255, 255, 255};
    
    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, color);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);

    color.g = color.b = 0;
    GXSetChanMatColor(GX_COLOR0, color);
}

/*---------------------------------------------------------------------------*
    Name:           MakeModelMtx
    
    Description:    computes a model matrix from 3 vectors representing an 
                    object's coordinate system.
                    
    Arguments:      xAxis   vector for the object's X axis
                    yAxis   vector for the object's Y axis
                    zAxis   vector for the object's Z axis
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void MakeModelMtx ( Vec xAxis, Vec yAxis, Vec zAxis, Mtx m )
{
    VECNormalize(&xAxis,&xAxis);
    VECNormalize(&yAxis,&yAxis);
    VECNormalize(&zAxis, &zAxis);

    m[0][0] = xAxis.x;
    m[1][0] = xAxis.y;
    m[2][0] = xAxis.z;

    m[0][1] = yAxis.x;
    m[1][1] = yAxis.y;
    m[2][1] = yAxis.z;

    m[0][2] = zAxis.x;
    m[1][2] = zAxis.y;
    m[2][2] = zAxis.z;

    m[0][3] = 0.0F;
    m[1][3] = 0.0F;
    m[2][3] = 0.0F;
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Animates the camera and object based on the joystick's 
                    state.
                    
    Arguments:      m   model matrix
                    v   view matrix
                    cameraLocScale  scale value for the camera's distance
                                    to the object.
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick ( Mtx v )
{
    Mtx rot;
    GXTexObj to;
    TEXDescriptorPtr tdp;

    u16 buttons = DEMOPadGetButton(0);
    u16 downs   = DEMOPadGetButtonDown(0);
    s8 stickX = DEMOPadGetStickX(0);
    s8 stickY = DEMOPadGetStickY(0);

    if(downs & PAD_BUTTON_X)
    {
        CurrentControl ++;
        if(CurrentControl > 4)
            CurrentControl = 0;

        switch(CurrentControl)
        {
            case 0:
                OSReport("\n\nCamera Control\n\n");
                break;
            case 1:
                OSReport("\n\nLight Position Control\n\n");
                break;
            case 2:
                OSReport("\n\nLight Parameter Control\n\n");
                break;
            case 3:
                OSReport("\n\nModel Select Control\n\n");
                break;
            case 4:
                OSReport("\n\nTexture Select Control\n\n");
                break;
        }
    }
    if(CurrentControl == 0)
    {
         // Move camera
        if(buttons & PAD_BUTTON_Y)
        {
            CameraLocScale *= .95F;
            if(CameraLocScale < 0.001F)
                CameraLocScale = 0.001F;
        }
        if(buttons & PAD_BUTTON_A)
            CameraLocScale *= 1.05F;        

        // Rotate camera
        if(stickX || stickY)
        {
            if(stickX)
             {
                if(stickX > 0) 
                    MTXRotAxisDeg(rot, &CamY, 3.0F);
                else 
                    MTXRotAxisDeg(rot, &CamY, -3.0F);

                MTXMultVec(rot, &CamX, &CamX);
                MTXMultVec(rot, &CamZ, &CamZ); 
            }
    
            if(stickY)
            {
                if(stickY > 0)
                    MTXRotAxisDeg(rot, &CamX, 3.0F);
                else 
                    MTXRotAxisDeg(rot, &CamX, -3.0F);

                MTXMultVec(rot, &CamY, &CamY);
                MTXMultVec(rot, &CamZ, &CamZ);          
            }
        }
    }
    else if(CurrentControl == 1)
    {
        // Rotate light
        if(stickX || stickY)
        {
            if(stickX)
             {
                if(stickX > 0) 
                    MTXRotAxisDeg(rot, &CamY, 3.0F);
                else 
                    MTXRotAxisDeg(rot, &CamY, -3.0F);

                MTXMultVec(rot, &LightX, &LightX);
                MTXMultVec(rot, &LightY, &LightY);
                MTXMultVec(rot, &LightZ, &LightZ); 
            }
    
            if(stickY)
            {
                if(stickY > 0)
                    MTXRotAxisDeg(rot, &CamX, 3.0F);
                else 
                    MTXRotAxisDeg(rot, &CamX, -3.0F);

                MTXMultVec(rot, &LightX, &LightX);
                MTXMultVec(rot, &LightY, &LightY);
                MTXMultVec(rot, &LightZ, &LightZ);          
            }
        }
        if(buttons & PAD_BUTTON_Y)
        {
            distance *= .95F;
            if(distance > -0.001F)
                distance = -0.001F;
        }
        if(buttons & PAD_BUTTON_A)
            distance *= 1.05F;
    }
    else if(CurrentControl == 2)
    {
        if(stickY > 0)
        {
            xmax *= .95F;
            if(xmax < 0.0001F)
                xmax = 0.0001F;
            ymin = -xmax;
            ymax = xmax;
            xmin = -xmax;
        }
        if(stickY < 0)
        {
            xmax *= 1.05F;
            ymin = -xmax;
            ymax = xmax;
            xmin = -xmax;
        }   
    }
    else if(CurrentControl == 3)
    {
        if(downs & PAD_BUTTON_B)
        {
            CurrentModel ++;
            if(CurrentModel > 6)
                CurrentModel = 0;
        }
    }
    else if(CurrentControl == 4)
    {
        if(downs & PAD_BUTTON_B)
        {
            CurrentTexture ++;
            if(CurrentTexture > 1)
                CurrentTexture = 0;
            switch(CurrentTexture)
            {
                case 1:
                    CurrentTevMode = GX_MODULATE;
                    break;
                case 0:
                    CurrentTevMode = GX_DECAL;
                    break;
            }
            tdp = TEXGet(tpl, CurrentTexture);

            GXInitTexObj(&to, 
                 tdp->textureHeader->data, 
                 tdp->textureHeader->width, 
                 tdp->textureHeader->height, 
                 (GXTexFmt)tdp->textureHeader->format,
                 GX_CLAMP, 
                 GX_CLAMP, 
                 GX_FALSE); 

            GXInitTexObjLOD(&to, 
                    tdp->textureHeader->minFilter, 
                    tdp->textureHeader->magFilter, 
                    tdp->textureHeader->minLOD, 
                    tdp->textureHeader->maxLOD, 
                    tdp->textureHeader->LODBias, 
                    GX_FALSE,
                    tdp->textureHeader->edgeLODEnable, 
                    GX_ANISO_1);

            GXLoadTexObj(&to, GX_TEXMAP0);
        }
    }

    MakeModelMtx(CamX, CamY, CamZ, v);  // Make a new model matrix
    MTXTranspose(v, v);
    MTXTrans(rot, 0.0F, 0.0F, -8.0F * CameraLocScale);
    MTXConcat(rot, v, v);

    MakeModelMtx(LightX, LightY, LightZ, lv);   // Make a new model matrix
    MTXTranspose(lv, lv);
    MTXTrans(rot, 0.0F, 0.0F, distance); 
    MTXConcat(rot, lv, lv);
    TextureLightInit(lv);
    MTXInverse(lv, lv);
    
}

static void DrawFrust ( void )
{
    float t = ffar / nnear;
    Mtx mv;

    MTXConcat(v, lv, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);

    GXBegin(GX_LINESTRIP, GX_VTXFMT0, 17);

        GXPosition3f32(xmin, ymin, -nnear); //n0
        GXColor4u8(255, 255, 0, 255);
 
        GXPosition3f32(xmax, ymin, -nnear);  //n1
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmax, ymax, -nnear);  //n2
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmin, ymax, -nnear);  //n3
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmin * t, ymax * t, -ffar);  //f3
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmax * t, ymax * t, -ffar);  //f2
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmax * t, ymin * t, -ffar);  //f1
        GXColor4u8(255, 255, 0, 255);
 
        GXPosition3f32(xmin * t, ymin * t, -ffar);  //f0
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmin, ymin, -nnear); //n0
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmax, ymin, -nnear);  //n1
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmax * t, ymin * t, -ffar);  //f1
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmin * t, ymin * t, -ffar);  //f0
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmin * t, ymax * t, -ffar);  //f3
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmax * t, ymax * t, -ffar);  //f2
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmax, ymax, -nnear);  //n2
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmin, ymax, -nnear);  //n3
        GXColor4u8(255, 255, 0, 255);

        GXPosition3f32(xmin, ymin, -nnear); //n0
        GXColor4u8(255, 255, 0, 255);
 
    GXEnd();
}

static void TextureLightInit ( Mtx rot )
{
    Mtx proj;
    Mtx mv;

    MTXLightFrustum(proj, ymin, ymax, xmin, xmax, nnear, 
                0.5F, 0.5F, 0.5F, 0.5F);

    MTXConcat(rot, m, mv);

    MTXConcat(proj, mv, proj);

    GXLoadTexMtxImm(proj, GX_TEXMTX0, GX_MTX3x4);
}

/*===========================================================================*/