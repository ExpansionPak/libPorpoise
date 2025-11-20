/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     smp-light.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Simple/smp-light.c $
    
    4     2/14/01 11:09a Hirose
    Changed to accept start button only to quit. Deleted obsolete ifdefs
    
    3     10/25/00 8:59p Hirose
    A flag fix MAC -> MACOS
    
    2     8/21/00 3:04p Hirose
    added singleframe test
    
    1     7/21/00 5:09p Hirose
    initial check in
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   smp-light
     A simple example of lighting
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
   Header files
 *---------------------------------------------------------------------------*/
#include <demo.h>
#include <math.h>

/*---------------------------------------------------------------------------*
   Macro definitions
 *---------------------------------------------------------------------------*/
#define PI    3.14159265358979323846F

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
void        main            ( void );
static void CameraInit      ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( Mtx v );
static void DrawModel       ( void );
static void PrintIntro      ( void );

/*---------------------------------------------------------------------------*
   Model data (octahedron)
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
   The macro ATTRIBUTE_ALIGN provides a convenient way to align initialized 
   arrays.  Alignment of vertex arrays to 32B IS NOT required, but may result 
   in a slight performance improvement.
 *---------------------------------------------------------------------------*/
f32 PosArray[] ATTRIBUTE_ALIGN(32) =
{
//      x,     y,     z       
     0.0f,  0.0f,  1.0f,    // face 0:0
     0.0f,  1.0f,  0.0f,    // face 0:1
     1.0f,  0.0f,  0.0f,    // face 0:2
     0.0f,  0.0f,  1.0f,    // face 1:0
    -1.0f,  0.0f,  0.0f,    // face 1:1
     0.0f,  1.0f,  0.0f,    // face 1:2
     0.0f,  0.0f,  1.0f,    // face 2:0
     0.0f, -1.0f,  0.0f,    // face 2:1
    -1.0f,  0.0f,  0.0f,    // face 2:2
     0.0f,  0.0f,  1.0f,    // face 3:0
     1.0f,  0.0f,  0.0f,    // face 3:1
     0.0f, -1.0f,  0.0f,    // face 3:2
     0.0f,  0.0f, -1.0f,    // face 4:0
     1.0f,  0.0f,  0.0f,    // face 4:1
     0.0f,  1.0f,  0.0f,    // face 4:2
     0.0f,  0.0f, -1.0f,    // face 5:0
     0.0f,  1.0f,  0.0f,    // face 5:1
    -1.0f,  0.0f,  0.0f,    // face 5:2
     0.0f,  0.0f, -1.0f,    // face 6:0
    -1.0f,  0.0f,  0.0f,    // face 6:1
     0.0f, -1.0f,  0.0f,    // face 6:2
     0.0f,  0.0f, -1.0f,    // face 7:0
     0.0f, -1.0f,  0.0f,    // face 7:1
     1.0f,  0.0f,  0.0f     // face 7:2
};

f32 NrmArray[] ATTRIBUTE_ALIGN(32) =
{
// For lighting, the vector doesn't have to be normalized.

//     nx,    ny,    nz
     1.0f,  1.0f,  1.0f,    // face 0
    -1.0f,  1.0f,  1.0f,    // face 1
    -1.0f, -1.0f,  1.0f,    // face 2
     1.0f, -1.0f,  1.0f,    // face 3
     1.0f,  1.0f, -1.0f,    // face 4
    -1.0f,  1.0f, -1.0f,    // face 5
    -1.0f, -1.0f, -1.0f,    // face 6
     1.0f, -1.0f, -1.0f,    // face 7
};

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/

// Color data for lighting
static GXColor AmbientColor  = { 0x40, 0x40, 0x40, 0x00 };
static GXColor MaterialColor = { 0x80, 0xD8, 0xFF, 0x00 };
static GXColor LightColor    = { 0xC0, 0xC0, 0xC0, 0x00 };

// Light position
static Vec LightPos = { 10.0F, 10.0F, 2.0F };

// Time counter
static u32  Ticks = 0;


/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
    Mtx         v;  // view matrix
    PADStatus   pad[PAD_MAX_CONTROLLERS]; // game pad state

    DEMOInit(NULL); // Init the OS, game pad, graphics and video.
    
    pad[0].button = 0;
    CameraInit(v);  // Initialize the camera.
    DrawInit();     // Initialize vertex formats and array pointers.

    PrintIntro();   // Print demo directions

    while(!(pad[0].button & PAD_BUTTON_MENU))
    {
		DEMOBeforeRender();
        DrawTick(v);       // Draw the model.
        DEMODoneRender();
        PADRead(pad);
        ++Ticks;           // Update time counter.
    }

    OSHalt("End of demo");
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
    Vec     up      = { 0.0F, 0.0F, 1.0F };
    Vec     camLoc  = { 2.0F, 3.0F, 1.0F };
    Vec     objPt   = { 0.0F, 0.0F, 0.0F };
    f32     left    = -0.050F;
    f32     top     = 0.0375F;
    f32     znear   = 0.1F;
    f32     zfar    = 10.0F;
    
    MTXFrustum(p, top, -top, left, -left, znear, zfar);
    GXSetProjection(p, GX_PERSPECTIVE);
    
    MTXLookAt(v, &camLoc, &up, &objPt);    
}

/*---------------------------------------------------------------------------*
    Name:           DrawInit
    
    Description:    Initializes the vertex attribute format and
                    array pointers.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit( void )
{ 
    // set up vertex attributes
    
    // Position has 3 elements (x,y,z), each of type f32
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    // Normal has 3 elements (x,y,z), each of type f32
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);

    // set up array pointers for indexed lookup

    // stride = 3 elements (x,y,z) each of type f32
    GXSetArray(GX_VA_POS, PosArray, 3*sizeof(f32));
    GXSetArray(GX_VA_NRM, NrmArray, 3*sizeof(f32));
}

/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draws the lit model
                    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( Mtx v )
{
    Mtx         mv;         // Modelview matrix.
    Mtx         mvi;        // Modelview matrix.
    Mtx         mr;         // Rotate matrix
    Vec         lpos;       // Light position.
    GXLightObj  myLight;    // Light object.

    //--------------------------------------------------
    //  Setting for using one color / no texture
    //--------------------------------------------------
    
    // it doesn't require any texture coord
    GXSetNumTexGens(0);
    // only require one TEV stage
    GXSetNumTevStages(1);
    // TEV should simply pass through color channel output
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);


    //--------------------------------------------------
    //  Lighting parameters
    //--------------------------------------------------

    // Light position
    lpos = LightPos;

    // If the light position is defined in the world space,
    // you should convert the coordinate because the lighting
    // HW is assuming view space coordinates.
    MTXMultVec(v, &lpos, &lpos);

    // Basical diffuse light requires position and color informations.
    GXInitLightPos(&myLight, lpos.x, lpos.y, lpos.z);
    GXInitLightColor(&myLight, LightColor);
    
    // Once parameter initialization is done, you should load the
    // light object into hardware to make it working.
    // In this case, the object data is loaded into GX_LIGHT0.
    GXLoadLightObjImm(&myLight, GX_LIGHT0);

    // Lighting channel control
    GXSetNumChans(1);    // number of active color channels
    GXSetChanCtrl(
        GX_COLOR0,       // color channel 0
        GX_ENABLE,       // enable channel (use lighting)
        GX_SRC_REG,      // use the register as ambient color source
        GX_SRC_REG,      // use the register as material color source
        GX_LIGHT0,       // use GX_LIGHT0
        GX_DF_CLAMP,     // diffuse function (CLAMP = normal setting)
        GX_AF_NONE );    // attenuation function (this case doesn't use)
    GXSetChanCtrl(
        GX_ALPHA0,       // alpha channel 0
        GX_DISABLE,      // not used in this program
        GX_SRC_REG,      // ambient source (N/A in this case)
        GX_SRC_REG,      // material source (N/A)
        GX_LIGHT0,       // light mask (N/A)
        GX_DF_NONE,      // diffuse function (N/A)
        GX_AF_NONE );    // attenuation function (N/A)

    // ambient color register
    GXSetChanAmbColor(GX_COLOR0A0, AmbientColor);
    // material color register
    GXSetChanMatColor(GX_COLOR0A0, MaterialColor);


    //--------------------------------------------------
    //  Geometry transformation
    //--------------------------------------------------

    // Calculate position transformation
    
    // model has a rotation about z axis
    MTXRotDeg(mr, 'z', Ticks);
    MTXConcat(v, mr, mv);
    GXLoadPosMtxImm(mv, GX_PNMTX0);
    
    // If you want to perform lighting, you must also set
    // normal transformation matrix. In general case, such
    // matrix can be obtained as inverse-transpose of the
    // position transform matrix.
    MTXInverse(mv, mvi);
    MTXTranspose(mvi, mv); 
    GXLoadNrmMtxImm(mv, GX_PNMTX0);


    //--------------------------------------------------
    //  Draw the model
    //--------------------------------------------------

    DrawModel();
}

/*---------------------------------------------------------------------------*
    Name:           DrawModel
    
    Description:    Draws the model (octahedron)
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawModel( void )
{
    u8    it;   // index of triangle
    u8    iv;   // index of vertex

    // sets up vertex descriptors
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    
    // send vertex data
    // normal is necessary for performing lighting.
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, 24);
    
    for ( it = 0 ; it < 8 ; ++it )
    {
        for ( iv = 0 ; iv < 3 ; ++iv )
        {
            GXPosition1x8((u8)( 3 * it + iv ));
            GXNormal1x8(it);   // same normal for each vertex on a triangle
        }
    }
    
    GXEnd();
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
    OSReport("********************************\n");
    OSReport("to quit:\n");
    OSReport("     hit the start button\n");
#ifdef MACOS
    OSReport("     click on the text output window\n");
    OSReport("     select quit from the menu or hit 'command q'\n");
    OSReport("     select 'don't save'\n");
#endif
    OSReport("********************************\n");
}

/*============================================================================*/
