/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     geo-vtx-fmt.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Geometry/geo-vtx-fmt.c $
    
    9     5/05/01 8:56p Hirose
    removed obsoleted flags
    
    8     7/01/00 2:26p Alligator
    add intro, change initial case for Rev A hw
    
    7     3/24/00 3:37p Carl
    Fixed problems relating to 8b scale bug.
    
    6     3/23/00 3:48p Hirose
    updated to use DEMOPad library
    
    5     3/23/00 9:56a Ryan
    update for Character Pipeline cleanup and function prefixing
    
    4     3/17/00 11:57a Carl
    Fixed single-frame setup stuff (again).
    
    3     3/13/00 4:34p Carl
    Eliminated normal shift stuff that wasn't supposed to be there.
    
    2     3/13/00 4:01p Carl
    Fixed code for proper single frame setup.
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
     
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     1/25/00 2:50p Carl
    Changed to standardized end of test message
    
    3     1/18/00 3:22p Hirose
    added GXSetNumChans(1)
    
    2     1/13/00 8:54p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    16    11/15/99 4:49p Yasu
    Change datafile name
    
    15    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    14    10/29/99 3:46p Hirose
    replaced GXSetTevStages(GXTevStageID) by GXSetNumTevStages(u8)
    
    13    10/22/99 4:38p Yasu
    Add GXSetTevStages and GXSetTevOrder
    
    12    9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    11    9/30/99 3:34p Ryan
    removed gxModels
    
    10    9/23/99 3:08p Ryan
    Added OSHalt at end of demo
    
    9     9/17/99 5:26p Ryan
    added new DEMO calls
    
    8     9/15/99 2:07p Ryan
    update to fix compiler warnings
    
    7     9/08/99 12:57p Ryan
    Added __SINGLEFRAME functionality
    
    6     8/30/99 4:35p Ryan
    
    5     8/27/99 3:38p Yasu
    Change the parameter of GXSetBlendMode()
    
    4     8/26/99 2:42p Yasu
    Replace GXSetColorMode() to  GXSetBlendMode()
    
    
    3     8/18/99 11:20a Ryan
    
    2     8/13/99 11:48a Ryan
    Added alpha blend stuff
    
    1     8/11/99 11:04a Ryan
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/

void        main            ( void );

static void CameraInit      ( void );
static void DrawInit        ( void );
static void DrawTick        ( void );
static void VertexLightInit ( void );

static void AnimTick        ( void );

static void SetColorParams      ( void );
static void SetTexCoordParams   ( void );
static void SetNormalParams     ( void );
static void SetPositionParams   ( void );

static void PrintIntro          ( void );

void DrawQuadCube( void );
void DrawTriCube( void );
void DrawPointCube( void );
void DrawLineCube( void );
void DrawLineStripCube( void );
void DrawTriStripCube( void );
void DrawTriFanCube( void );

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/
#define SIDE 50
#define WOOD1_TEX_ID        5

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
Mtx v;

TEXPalettePtr tpl = 0;

u32 rot;

float FloatVert[] ATTRIBUTE_ALIGN(32) =  
                     {  -SIDE, SIDE, -SIDE,
                        -SIDE, SIDE, SIDE,
                        -SIDE, -SIDE, SIDE,
                        -SIDE, -SIDE, -SIDE,
                        SIDE, SIDE, -SIDE,
                        SIDE, -SIDE, -SIDE,
                        SIDE, -SIDE, SIDE,
                        SIDE, SIDE, SIDE
                    };
s16 Vert16[] ATTRIBUTE_ALIGN(32) = 
                    {   -SIDE, SIDE, -SIDE,
                        -SIDE, SIDE, SIDE,
                        -SIDE, -SIDE, SIDE,
                        -SIDE, -SIDE, -SIDE,
                        SIDE, SIDE, -SIDE,
                        SIDE, -SIDE, -SIDE,
                        SIDE, -SIDE, SIDE,
                        SIDE, SIDE, SIDE
                    };
s8 Vert8[] ATTRIBUTE_ALIGN(32) =        
                    {   -SIDE, SIDE, -SIDE,
                        -SIDE, SIDE, SIDE,
                        -SIDE, -SIDE, SIDE,
                        -SIDE, -SIDE, -SIDE,
                        SIDE, SIDE, -SIDE,
                        SIDE, -SIDE, -SIDE,
                        SIDE, -SIDE, SIDE,
                        SIDE, SIDE, SIDE
                    };

float FloatNorm[] ATTRIBUTE_ALIGN(32) = 
                    {   -1.0F, 0.0F, 0.0F,
                        1.0F, 0.0F, 0.0F,
                        0.0F, -1.0F, 0.0F,
                        0.0F, 1.0F, 0.0F,
                        0.0F, 0.0F, -1.0F,
                        0.0F, 0.0F, 1.0F                        
                    };
s16 Norm16[] ATTRIBUTE_ALIGN(32) =      
                    {   -1, 0, 0,
                        1, 0, 0,
                        0, -1, 0,
                        0, 1, 0,
                        0, 0, -1,
                        0, 0, 1                     
                    };
s8 Norm8[] ATTRIBUTE_ALIGN(32) =        
                    {   -1, 0, 0,
                        1, 0, 0,
                        0, -1, 0,
                        0, 1, 0,
                        0, 0, -1,
                        0, 0, 1                     
                    };

u8  ColorRGBA8[] ATTRIBUTE_ALIGN(32) =  
                        {   255, 0, 0, 255, 
                            255, 0, 0, 192, 
                            255, 0, 0, 128, 
                            255, 0, 0, 64, 
                            255, 0, 0, 0};      //GX_RGBA8
u8  ColorRGBA6[] ATTRIBUTE_ALIGN(32) =  
                        {   0xFC, 0x00, 0x3F, 
                            0xFC, 0x00, 0x30,
                            0xFC, 0x00, 0x20,
                            0xFC, 0x00, 0x10,
                            0xFC, 0x00, 0x00};  //GX_RGBA6
u8  ColorRGBA4[] ATTRIBUTE_ALIGN(32) =  
                        {   0xF0, 0x0F, 
                            0xF0, 0x0C,
                            0xF0, 0x08,
                            0xF0, 0x04,
                            0xF0, 0x00};            //GX_RGBA4
u8  ColorRGBX8[] ATTRIBUTE_ALIGN(32) =  
                        {   255, 0, 0, 0,
                            255, 0, 0, 0,
                            255, 0, 0, 0,
                            255, 0, 0, 0,
                            255, 0, 0, 0};      //GX_RGBX8
u8  ColorRGB8[] ATTRIBUTE_ALIGN(32) =  
                        {   255, 0, 0,
                            255, 0, 0,
                            255, 0, 0,
                            255, 0, 0,
                            255, 0, 0};             //GX_RGB8
u8  ColorRGB565[] ATTRIBUTE_ALIGN(32) =  
                        {   0xF8, 0x00,
                            0xF8, 0x00,
                            0xF8, 0x00,
                            0xF8, 0x00,
                            0xF8, 0x00};            //GX_RGB565


float FloatTex[] ATTRIBUTE_ALIGN(32) =  
                    {   0.0F, 0.0F, 
                        1.0F, 0.0F,
                        1.0F, 1.0F, 
                        0.0F, 1.0F
                    };
u16 Tex16[] ATTRIBUTE_ALIGN(32) =       
                    {   0, 0, 
                        1, 0,
                        1, 1, 
                        0, 1
                    };
u8 Tex8[] ATTRIBUTE_ALIGN(32) =         
                    {   0, 0, 
                        1, 0,
                        1, 1, 
                        0, 1
                    };


u8  CurrentControl = 0;
u8  PositionControl = 0;
u8  ColorControl = 1;
u8  NormalControl = 0;
u8  TexCoordControl = 0;

GXBool  LightingOn = GX_TRUE;
GXColorSrc MatSrc = GX_SRC_VTX;

u8  PositionShift = 0;
u8  TexCoordShift = 0;


/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    DEMOInit(NULL);
           
    DrawInit();         // Define my vertex formats and set array pointers.

    VertexLightInit();

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
static void DrawInit( void )
{
    GXTexObj to;

    SetPositionParams();
    SetColorParams();
    SetNormalParams();
    SetTexCoordParams();

    CameraInit();   // Initialize the camera.

    TEXGetPalette(&tpl, "gxTextrs.tpl");

    TEXGetGXTexObjFromPalette(tpl, &to, WOOD1_TEX_ID);

    GXLoadTexObj(&to, GX_TEXMAP0);

    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, 
                       GX_LO_CLEAR);
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
    DrawQuadCube();
    DrawTriCube();
    DrawPointCube();
    DrawLineCube();
    DrawLineStripCube();
    DrawTriStripCube();
    DrawTriFanCube();
}

/*---------------------------------------------------------------------------*/
static void VertexLightInit ( void )
{
    GXLightObj MyLight;
    GXColor color = {255, 255, 255, 255};
    
    GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, color);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);

    GXSetChanMatColor(GX_COLOR0A0, color);
    //GXSetChanMatColor(GX_ALPHA0, color);

    /*GXSetChanCtrl(
                GX_ALPHA0,
                GX_FALSE,   // enable channel
                GX_SRC_REG, // amb source
                GX_SRC_REG, // mat source
                GX_LIGHT0,  // light mask
                GX_DF_CLAMP,// diffuse function
                GX_AF_NONE);*/
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
    u16 buttons = DEMOPadGetButtonDown(0);
    s8 stickX = DEMOPadGetStickX(0);
    s8 stickY = DEMOPadGetStickY(0);

    rot ++;
    if(rot > 719)
        rot = 0;

    if(buttons & PAD_BUTTON_X)
    {
        CurrentControl ++;
        if(CurrentControl > 5)
            CurrentControl = 0;

        switch(CurrentControl)
        {
            case 0:
                OSReport("\n\nPosition Control\n\n");
                break;
            case 1:
                OSReport("\n\nColor Control\n\n");
                break;
            case 2:
                OSReport("\n\nNormal Control\n\n");
                break;
            case 3:
                OSReport("\n\nTextureCoord Control\n\n");
                break;
            case 4:
                OSReport("\n\nPosition Shift Control\n\n");
                break;
            case 5:
                OSReport("\n\nTextureCoord Shift Control\n\n");
                break;
        }
    }
    if(CurrentControl == 0)
    {
        if(buttons & PAD_BUTTON_B)
        {
            PositionControl ++;
            if(PositionControl > 8)
                PositionControl = 0;

            switch(PositionControl)
            {
                case 0:
                    OSReport("Position on - 16 bit indexed floating point data\n");
                    break;
                case 1:
                    OSReport("Position on - 16 bit indexed 16 bit data\n");
                    break;
                case 2:
                    OSReport("Position on - 16 bit indexed 8 bit data\n");
                    break;
                case 3:
                    OSReport("Position on - 8 bit indexed floating point data\n");
                    break;
                case 4:
                    OSReport("Position on - 8 bit indexed 16 bit data\n");
                    break;
                case 5:
                    OSReport("Position on - 8 bit indexed 8 bit data\n");
                    break;
                case 6:
                    OSReport("Position on - Direct floating point data\n");
                    break;
                case 7:
                    OSReport("Position on - Direct 16 bit data\n");
                    break;
                case 8:
                    OSReport("Position on - Direct 8 bit data\n");
                    break;
            }

            SetPositionParams();
        }                   
    }
    else if(CurrentControl == 1)
    {
        if(buttons & PAD_BUTTON_B)
        {
            ColorControl ++;
            if(ColorControl > 20)
                ColorControl = 0;

            switch(ColorControl)
            {
                case 0:
                    OSReport("Color off\n");
                    break;
                case 1:
                    OSReport("Color on - 16 bit indexed RGBA8\n");
                    break;
                case 2:
                    OSReport("Color on - 16 bit indexed RGBA6\n");
                    break;
                case 3:
                    OSReport("Color on - 16 bit indexed RGBA4\n");
                    break;
                case 4:
                    OSReport("Color on - 16 bit indexed RGBX8\n");
                    break;
                case 5:
                    OSReport("Color on - 16 bit indexed RGB8\n");
                    break;
                case 6:
                    OSReport("Color on - 16 bit indexed RGB565\n");
                    break;
                case 7:
                    OSReport("Color on - 8 bit indexed RGBA8\n");
                    break;
                case 8:
                    OSReport("Color on - 8 bit indexed RGBA6\n");
                    break;
                case 9:
                    OSReport("Color on - 8 bit indexed RGBA4\n");
                    break;
                case 10:
                    OSReport("Color on - 8 bit indexed RGBX8\n");
                    break;
                case 11:
                    OSReport("Color on - 8 bit indexed RGB8\n");
                    break;
                case 12:
                    OSReport("Color on - 8 bit indexed RGB565\n");
                    break;

                case 13:
                    OSReport("Color on - direct RGBA8 (4 u8's)\n");
                    break;
                case 14:
                    OSReport("Color on - direct RGBA8 (1 u32)\n");
                    break;

                case 15:
                    OSReport("Color on - direct RGBA6 (3 u8's)\n");
                    break;
                case 16:
                    OSReport("Color on - direct RGBA4 (1 u16)\n");
                    break;

                case 17:
                    OSReport("Color on - direct RGBX8 (4 u8's)\n");
                    break;
                case 18:
                    OSReport("Color on - direct RGBX8 (1 u32)\n");
                    break;

                case 19:
                    OSReport("Color on - direct RGB8 (3 u8's)\n");
                    break;
                case 20:
                    OSReport("Color on - direct RGB565 (1 u16)\n");
                    break;
            }

            SetColorParams();
        }                   
    }
    else if(CurrentControl == 2)
    {
        if(buttons & PAD_BUTTON_B)
        {
            NormalControl ++;
            if(NormalControl > 9)
                NormalControl = 0;

            switch(NormalControl)
            {
                case 0:
                    OSReport("Normal off\n");
                    break;
                case 1:
                    OSReport("Normal on - 16 bit indexed floating point data\n");
                    break;
                case 2:
                    OSReport("Normal on - 16 bit indexed 16 bit data\n");
                    break;
                case 3:
                    OSReport("Normal on - 16 bit indexed 8 bit data\n");
                    break;
                case 4:
                    OSReport("Normal on - 8 bit indexed floating point data\n");
                    break;
                case 5:
                    OSReport("Normal on - 8 bit indexed 16 bit data\n");
                    break;
                case 6:
                    OSReport("Normal on - 8 bit indexed 8 bit data\n");
                    break;
                case 7:
                    OSReport("Normal on - Direct floating point data\n");
                    break;
                case 8:
                    OSReport("Normal on - Direct 16 bit data\n");
                    break;
                case 9:
                    OSReport("Normal on - Direct 8 bit data\n");
                    break;
            }

            SetNormalParams();
        }                   
    }
    else if(CurrentControl == 3)
    {
        if(buttons & PAD_BUTTON_B)
        {
            TexCoordControl ++;
            if(TexCoordControl > 9)
                TexCoordControl = 0;

            switch(TexCoordControl)
            {
                case 0:
                    OSReport("TexCoord off\n");
                    break;
                case 1:
                    OSReport("TexCoord on - 16 bit indexed floating point data\n");
                    break;
                case 2:
                    OSReport("TexCoord on - 16 bit indexed 16 bit data\n");
                    break;
                case 3:
                    OSReport("TexCoord on - 16 bit indexed 8 bit data\n");
                    break;
                case 4:
                    OSReport("TexCoord on - 8 bit indexed floating point data\n");
                    break;
                case 5:
                    OSReport("TexCoord on - 8 bit indexed 16 bit data\n");
                    break;
                case 6:
                    OSReport("TexCoord on - 8 bit indexed 8 bit data\n");
                    break;
                case 7:
                    OSReport("TexCoord on - Direct floating point data\n");
                    break;
                case 8:
                    OSReport("TexCoord on - Direct 16 bit data\n");
                    break;
                case 9:
                    OSReport("TexCoord on - Direct 8 bit data\n");
                    break;
            }

            SetTexCoordParams();
        }                   
    }
    else if(CurrentControl == 4)
    {
        if(buttons & PAD_BUTTON_B)
        {
            PositionShift ++;
            if(PositionShift > 16)
                PositionShift = 0;

            OSReport("Position shift  - %d\n", PositionShift);
            
            SetPositionParams();
        }                   
    }
    else if(CurrentControl == 5)
    {
        if(buttons & PAD_BUTTON_B)
        {
            TexCoordShift ++;
            if(TexCoordShift > 16)
                TexCoordShift = 0;

            OSReport("TexCoord shift  - %d\n", TexCoordShift);
            
            SetTexCoordParams();
        }                   
    }
    
}

/*---------------------------------------------------------------------------*/
static void SetColorParams( void )
{
    GXSetNumChans(1);

    switch(ColorControl)
    {
        case 0:
            GXSetVtxDesc(GX_VA_CLR0, GX_NONE);
            MatSrc = GX_SRC_REG;
            GXSetChanCtrl(
                GX_COLOR0,
                LightingOn, // enable channel
                GX_SRC_REG, // amb source
                MatSrc,     // mat source
                GX_LIGHT0,  // light mask
                GX_DF_CLAMP,// diffuse function
                GX_AF_NONE);
            GXSetChanCtrl(
                GX_ALPHA0,
                GX_FALSE,   // enable channel
                GX_SRC_REG, // amb source
                GX_SRC_REG, // mat source
                GX_LIGHT0,  // light mask
                GX_DF_CLAMP,// diffuse function
                GX_AF_NONE);
            break;
        case 1:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
            GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
            GXSetArray(GX_VA_CLR0, ColorRGBA8, 4);
            MatSrc = GX_SRC_VTX;
            GXSetChanCtrl(
                GX_COLOR0,
                LightingOn, // enable channel
                GX_SRC_REG, // amb source
                MatSrc,     // mat source
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
            break;
        case 2:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA6, 0);
            GXSetArray(GX_VA_CLR0, ColorRGBA6, 3);
            break;
        case 3:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0);
            GXSetArray(GX_VA_CLR0, ColorRGBA4, 2);
            break;
        case 4:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGBX8, 0);
            GXSetArray(GX_VA_CLR0, ColorRGBX8, 4);
            break;
        case 5:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
            GXSetArray(GX_VA_CLR0, ColorRGB8, 3);
            break;
        case 6:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB565, 0);
            GXSetArray(GX_VA_CLR0, ColorRGB565, 2);
            break;
        case 7:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
            GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
            GXSetArray(GX_VA_CLR0, ColorRGBA8, 4);
            break;
        case 8:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA6, 0);
            GXSetArray(GX_VA_CLR0, ColorRGBA6, 3);
            break;
        case 9:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0);
            GXSetArray(GX_VA_CLR0, ColorRGBA4, 2);
            break;
        case 10:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGBX8, 0);
            GXSetArray(GX_VA_CLR0, ColorRGBX8, 4);
            break;
        case 11:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
            GXSetArray(GX_VA_CLR0, ColorRGB8, 3);
            break;
        case 12:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB565, 0);
            GXSetArray(GX_VA_CLR0, ColorRGB565, 2);
            break;
        case 13:
        case 14:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
            GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
            break;
        case 15:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA6, 0);
            break;
        case 16:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0);
            break;
        case 17:
        case 18:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGBX8, 0);
            break;
        case 19:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB8, 0);
            break;
        case 20:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGB565, 0);
            break;
    }
}

/*---------------------------------------------------------------------------*/
static void SetTexCoordParams( void )
{
    switch(TexCoordControl)
    {
        case 0:
            GXSetVtxDesc(GX_VA_TEX0, GX_NONE);
            GXSetNumTexGens(0);
            GXSetNumTevStages(1);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
            break;
        case 1:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, TexCoordShift);
            GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
            GXSetArray(GX_VA_TEX0, FloatTex, 8);
            GXSetNumTexGens(1);
            GXSetNumTevStages(1);
            GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
            GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
            break;
        case 2:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, TexCoordShift);
            GXSetArray(GX_VA_TEX0, Tex16, 4);
            break;
        case 3:
#ifdef BUG_NO_8b_SCALE
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
#else
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, TexCoordShift);
#endif
            GXSetArray(GX_VA_TEX0, Tex8, 2);
            break;
        case 4:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, TexCoordShift);
            GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
            GXSetArray(GX_VA_TEX0, FloatTex, 8);
            break;
        case 5:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, TexCoordShift);
            GXSetArray(GX_VA_TEX0, Tex16, 4);
            break;
        case 6:
#ifdef BUG_NO_8b_SCALE
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
#else
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, TexCoordShift);
#endif
            GXSetArray(GX_VA_TEX0, Tex8, 2);
            break;
        case 7:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, TexCoordShift);
            GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
            break;
        case 8:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U16, TexCoordShift);
            break;
        case 9:
#ifdef BUG_NO_8b_SCALE
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, 0);
#else
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_U8, TexCoordShift);
#endif
            break;
    }
}

/*---------------------------------------------------------------------------*/
static void SetNormalParams( void )
{
    switch(NormalControl)
    {
        case 0:
            GXSetVtxDesc(GX_VA_NRM, GX_NONE);
            LightingOn = GX_FALSE;
            GXSetChanCtrl(
                GX_COLOR0,
                LightingOn, // enable channel
                GX_SRC_REG, // amb source
                MatSrc,     // mat source
                GX_LIGHT0,  // light mask
                GX_DF_CLAMP,// diffuse function
                GX_AF_NONE);
            break;
        case 1:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
            GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
            GXSetArray(GX_VA_NRM, FloatNorm, 12);
            LightingOn = GX_TRUE;
            GXSetChanCtrl(
                GX_COLOR0,
                LightingOn, // enable channel
                GX_SRC_REG, // amb source
                MatSrc,     // mat source
                GX_LIGHT0,  // light mask
                GX_DF_CLAMP,// diffuse function
                GX_AF_NONE);
            break;
        case 2:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 0);
            GXSetArray(GX_VA_NRM, Norm16, 6);
            break;
        case 3:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);
            GXSetArray(GX_VA_NRM, Norm8, 3);
            break;
        case 4:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
            GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
            GXSetArray(GX_VA_NRM, FloatNorm, 12);
            break;
        case 5:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 0);
            GXSetArray(GX_VA_NRM, Norm16, 6);
            break;
        case 6:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);
            GXSetArray(GX_VA_NRM, Norm8, 3);
            break;
        case 7:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
            GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
            break;
        case 8:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S16, 0);
            break;
        case 9:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);
            break;
    }
}

/*---------------------------------------------------------------------------*/
static void SetPositionParams( void )
{
    switch(PositionControl)
    {
        case 0:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, PositionShift);
            GXSetVtxDesc (GX_VA_POS, GX_INDEX16);
            GXSetArray(GX_VA_POS, FloatVert, 12);
            break;
        case 1:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, PositionShift);
            GXSetArray(GX_VA_POS, Vert16, 6);
            break;
        case 2:
#ifdef BUG_NO_8b_SCALE
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, 0);
#else
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, PositionShift);
#endif
            GXSetArray(GX_VA_POS, Vert8, 3);
            break;
        case 3:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, PositionShift);
            GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
            GXSetArray(GX_VA_POS, FloatVert, 12);
            break;
        case 4:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, PositionShift);
            GXSetArray(GX_VA_POS, Vert16, 6);
            break;
        case 5:
#ifdef BUG_NO_8b_SCALE
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, 0);
#else
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, PositionShift);
#endif
            GXSetArray(GX_VA_POS, Vert8, 3);
            break;
        case 6:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, PositionShift);
            GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
            break;
        case 7:
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, PositionShift);
            break;
        case 8:
#ifdef BUG_NO_8b_SCALE
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, 0);
#else
            GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S8, PositionShift);
#endif
            break;
    }
}

/*---------------------------------------------------------------------------*
    Name:           ParameterInit
    
    Description:    Initialize parameters for single frame display              
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
#if 0
static void ParameterInit( u32 id )
{
    u8 tmp;

    PositionControl = (u8)(id & 0x0000000F);
    ColorControl = (u8)((id >> 4) & 0x0000001F);
    // unused bits: 9-11
    NormalControl = (u8)((id >> 12) & 0x0000000F);
    TexCoordControl = (u8)((id >> 16) & 0x0000000F);
    PositionShift = (u8)((id >> 20) & 0x0000001F);
    TexCoordShift = (u8)((id >> 25) & 0x0000001F);
    // unused bits: 30-31

    rot = 60;

    switch(PositionControl)
    {
        case 0:
            OSReport("Position on - 16 bit indexed floating point data\n");
            break;
        case 1:
            OSReport("Position on - 16 bit indexed 16 bit data\n");
            break;
        case 2:
            OSReport("Position on - 16 bit indexed 8 bit data\n");
            break;
        case 3:
            OSReport("Position on - 8 bit indexed floating point data\n");
            break;
        case 4:
            OSReport("Position on - 8 bit indexed 16 bit data\n");
            break;
        case 5:
            OSReport("Position on - 8 bit indexed 8 bit data\n");
            break;
        case 6:
            OSReport("Position on - Direct floating point data\n");
            break;
        case 7:
            OSReport("Position on - Direct 16 bit data\n");
            break;
        case 8:
            OSReport("Position on - Direct 8 bit data\n");
            break;
        default:
            OSHalt("invalid setting for __SINGLEFRAME for position\n\n");
            break;
    }
    
    switch(ColorControl)
    {
        case 0:
            OSReport("Color off\n");
            break;
        case 1:
            OSReport("Color on - 16 bit indexed RGBA8\n");
            break;
        case 2:
            OSReport("Color on - 16 bit indexed RGBA6\n");
            break;
        case 3:
            OSReport("Color on - 16 bit indexed RGBA4\n");
            break;
        case 4:
            OSReport("Color on - 16 bit indexed RGBX8\n");
            break;
        case 5:
            OSReport("Color on - 16 bit indexed RGB8\n");
            break;
        case 6:
            OSReport("Color on - 16 bit indexed RGB565\n");
            break;
        case 7:
            OSReport("Color on - 8 bit indexed RGBA8\n");
            break;
        case 8:
            OSReport("Color on - 8 bit indexed RGBA6\n");
            break;
        case 9:
            OSReport("Color on - 8 bit indexed RGBA4\n");
            break;
        case 10:
            OSReport("Color on - 8 bit indexed RGBX8\n");
            break;
        case 11:
            OSReport("Color on - 8 bit indexed RGB8\n");
            break;
        case 12:
            OSReport("Color on - 8 bit indexed RGB565\n");
            break;
        case 13:
            OSReport("Color on - direct RGBA8 (4 u8's)\n");
            break;
        case 14:
            OSReport("Color on - direct RGBA8 (1 u32)\n");
            break;
        case 15:
            OSReport("Color on - direct RGBA6 (3 u8's)\n");
            break;
        case 16:
            OSReport("Color on - direct RGBA4 (1 u16)\n");
            break;
        case 17:
            OSReport("Color on - direct RGBX8 (4 u8's)\n");
            break;
        case 18:
            OSReport("Color on - direct RGBX8 (1 u32)\n");
            break;
        case 19:
            OSReport("Color on - direct RGB8 (3 u8's)\n");
            break;
        case 20:
            OSReport("Color on - direct RGB565 (1 u16)\n");
            break;
        default:
            OSHalt("invalid setting for __SINGLEFRAME for Color\n\n");
            break;
    }
    
    switch(NormalControl)
    {
        case 0:
            OSReport("Normal off\n");
            break;
        case 1:
            OSReport("Normal on - 16 bit indexed floating point data\n");
            break;
        case 2:
            OSReport("Normal on - 16 bit indexed 16 bit data\n");
            break;
        case 3:
            OSReport("Normal on - 16 bit indexed 8 bit data\n");
            break;
        case 4:
            OSReport("Normal on - 8 bit indexed floating point data\n");
            break;
        case 5:
            OSReport("Normal on - 8 bit indexed 16 bit data\n");
            break;
        case 6:
            OSReport("Normal on - 8 bit indexed 8 bit data\n");
            break;
        case 7:
            OSReport("Normal on - Direct floating point data\n");
            break;
        case 8:
            OSReport("Normal on - Direct 16 bit data\n");
            break;
        case 9:
            OSReport("Normal on - Direct 8 bit data\n");
            break;
        default:
            OSHalt("invalid setting for __SINGLEFRAME for normal\n\n");
            break;
    }
    
    switch(TexCoordControl)
    {
        case 0:
            OSReport("TexCoord off\n");
            break;
        case 1:
            OSReport("TexCoord on - 16 bit indexed floating point data\n");
            break;
        case 2:
            OSReport("TexCoord on - 16 bit indexed 16 bit data\n");
            break;
        case 3:
            OSReport("TexCoord on - 16 bit indexed 8 bit data\n");
            break;
        case 4:
            OSReport("TexCoord on - 8 bit indexed floating point data\n");
            break;
        case 5:
            OSReport("TexCoord on - 8 bit indexed 16 bit data\n");
            break;
        case 6:
            OSReport("TexCoord on - 8 bit indexed 8 bit data\n");
            break;
        case 7:
            OSReport("TexCoord on - Direct floating point data\n");
            break;
        case 8:
            OSReport("TexCoord on - Direct 16 bit data\n");
            break;
        case 9:
            OSReport("TexCoord on - Direct 8 bit data\n");
            break;
        default:
            OSHalt("invalid setting for __SINGLEFRAME for texCoord\n\n");
            break;
    }
    
    OSReport("Position shift  - %d\n", PositionShift);  
    OSReport("TexCoord shift  - %d\n", TexCoordShift);  

    // Extra code needed to set these controls correctly...
    // One must cycle through in order to reach the correct setting.

    tmp = PositionControl;
    for(PositionControl = 0; PositionControl < tmp; PositionControl++)
    {
        SetPositionParams();
    }
    SetPositionParams();

    tmp = ColorControl;
    for(ColorControl = 0; ColorControl < tmp; ColorControl++) 
    {
        SetColorParams();
    }
    SetColorParams();

    tmp = NormalControl;
    for(NormalControl = 0; NormalControl < tmp; NormalControl++)
    {
        SetNormalParams();
    }
    SetNormalParams();

    tmp = TexCoordControl;
    for(TexCoordControl = 0; TexCoordControl < tmp; TexCoordControl++)
    {
        SetTexCoordParams();
    }
    SetTexCoordParams();
}
#endif

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
    OSReport("geo-vtx-fmt: test combinations of vertex formats\n");
    OSReport("************************************************\n");
    OSReport("to quit hit the menu button\n");
    OSReport("\n");
    OSReport("  X button     : change attribute\n");
    OSReport("  B button     : change current attribute format\n");
    OSReport("************************************************\n\n");
}

