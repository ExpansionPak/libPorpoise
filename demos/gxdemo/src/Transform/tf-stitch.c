/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     geo-stitch.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Transform/tf-stitch.c $
    
    4     7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    3     5/10/01 5:07a Hirose
    removed obsolete flags
    
    2     3/24/00 5:46p Hirose
    changed to use DEMOPad library
    
    1     3/06/00 12:14p Alligator
    move from tests/gx and rename
    
    8     3/03/00 4:21p Alligator
    integrate with ArtX source
    
    7     2/25/00 1:34p Carl
    Fixed default matrix problem.
    
    6     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    5     1/25/00 3:00p Carl
    Changed to standardized end of test message
    
    4     1/21/00 1:04p Ryan
    update to fix matrix loads
    
    3     1/18/00 6:14p Alligator
    fix to work with new GXInit defaults
    
    2     1/13/00 8:55p Danm
    Added GXRenderModeObj * parameter to DEMOInit()
    
    11    11/17/99 1:25p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    10    11/12/99 4:30p Yasu
    Add GXSetNumTexGens(0) in GX_PASSCLR mode
    
    9     11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    8     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    7     9/23/99 3:05p Ryan
    added OSHalt at end of demo
    
    6     9/17/99 5:26p Ryan
    added new DEMO calls
    
    5     9/15/99 2:20p Ryan
    update to fix compiler warnings
    
    4     9/10/99 2:08p Ryan
    Added data $ flush after coordinate generation
    
    3     9/01/99 12:18p Ryan
    Aded __SINGLEFRAME crap
    
    2     8/19/99 12:12p Ryan
    
    3     7/28/99 10:57a Alligator
    
    1     7/23/99 12:32p Alligator
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

#define PI    3.14159265358979323846F

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
 
void        main            ( void );
static void CameraInit      ( Mtx v );
static void DrawInit        ( void );
static void DrawTick        ( Mtx v );
static void AnimTick        ( void );

static float ComputeScale 	( float position );

/*---------------------------------------------------------------------------*
  Model Data
 *---------------------------------------------------------------------------*/
#define Black   MyColors[0]
#define White   MyColors[5]
#define Red     MyColors[1]
#define Green   MyColors[2]
#define Blue    MyColors[3]
#define Gray    MyColors[4]


u32 MyColors[] ATTRIBUTE_ALIGN(32) = {
    0x00ffffff,  
    0xff0000ff,  
    0xffff00ff,  
    0x0000ffff,  
	0x08ff08ff,
	0xff00ffff,
    
	0x808080ff,	 
	0xffffffff,	 
	0xffff00ff,
	0x00ff00ff}; 

GXLightObj MyLight;

typedef struct {
    GXPosNrmMtx mtx;
    f32         xlate;
	f32			recomputeNormalMtx;
} Segment;

Segment segs[] ATTRIBUTE_ALIGN(32) = {
		{GX_PNMTX0, -540.0F, 0},
        {GX_PNMTX1, -520.0F, 0},
        {GX_PNMTX2, -500.0F, 0},
        {GX_PNMTX3, -480.0F, 0},
        {GX_PNMTX4, -460.0F, 0},
        {GX_PNMTX5, -440.0F, 0},
		{GX_PNMTX6, -420.0F, 0},
		{GX_PNMTX7, -400.0F, 0},
		{GX_PNMTX8, -380.0F, 0},

		{GX_PNMTX9, -360.0F, 0},

		{GX_PNMTX0, -340.0F, 0},
        {GX_PNMTX1, -320.0F, 0},
        {GX_PNMTX2, -300.0F, 0},
        {GX_PNMTX3, -280.0F, 0},
        {GX_PNMTX4, -260.0F, 0},
		{GX_PNMTX5, -240.0F, 0},
		{GX_PNMTX6, -220.0F, 0},
		{GX_PNMTX7, -200.0F, 0},

		{GX_PNMTX8, -180.0F, 0},

		{GX_PNMTX9, -160.0F, 0},
        {GX_PNMTX0, -140.0F, 0},
        {GX_PNMTX1, -120.0F, 0},
        {GX_PNMTX2, -100.0F, 0},
        {GX_PNMTX3, -80.0F, 0},
		{GX_PNMTX4, -60.0F, 0},
		{GX_PNMTX5, -40.0F, 0},
		{GX_PNMTX6, -20.0F, 0},

		{GX_PNMTX7, 0.0F, 0},

		{GX_PNMTX8, 20.0F, 0},
        {GX_PNMTX9, 40.0F, 0},
        {GX_PNMTX0, 60.0F, 0},
        {GX_PNMTX1, 80.0F, 0},
        {GX_PNMTX2, 100.0F, 0},
		{GX_PNMTX3, 120.0F, 0},
		{GX_PNMTX4, 140.0F, 0},
		{GX_PNMTX5, 160.0F, 0},

		{GX_PNMTX6, 180.0F, 0},
				 
		{GX_PNMTX7, 200.0F, 0},
        {GX_PNMTX8, 220.0F, 0},
        {GX_PNMTX9, 240.0F, 0},
        {GX_PNMTX0, 260.0F, 0},
        {GX_PNMTX1, 280.0F, 0},
		{GX_PNMTX2, 300.0F, 0},
		{GX_PNMTX3, 320.0F, 0},
		{GX_PNMTX4, 340.0F, 0},

		{GX_PNMTX5, 360.0F, 0},

		{GX_PNMTX6, 380.0F, 0},
        {GX_PNMTX7, 400.0F, 0},
        {GX_PNMTX8, 420.0F, 0},
        {GX_PNMTX9, 440.0F, 0},
        {GX_PNMTX0, 460.0F, 0},
		{GX_PNMTX1, 480.0F, 0},
		{GX_PNMTX2, 500.0F, 0},
		{GX_PNMTX3, 520.0F, 0},

		{GX_PNMTX4, 540.0F, 0}};

f32 MyVert[200] ATTRIBUTE_ALIGN(32);

float rot = 0;
float rot2;
u8 direction = 0;

float BallPosition = 800;
float BallStep = -10;

Mtx         v;   // view matrix

/*---------------------------------------------------------------------------*/
static void ComputeNormalRotMtx ( Segment *seg, u32 idx );

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/

void main ( void )
{
    DEMOInit(NULL);
    
    CameraInit(v); // Initialize the camera.  
    DrawInit();    // Define my vertex formats and set array pointers.

    while(!(DEMOPadGetButton(0) & PAD_BUTTON_MENU))
    {   
		DEMOBeforeRender();

        DrawTick(v);        // Draw the model.

		DEMODoneRender();

        DEMOPadRead();      // Update pad status.
        AnimTick();         // Update animation.
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
    Vec     up      = {0.0F, 1.0F, 0.0F};
    Vec     camLoc  = {0.0F, 0.0F, 1200.0F};
    Vec     objPt   = {0.0F, 0.0F, 0.0F};
    f32     left    = 240.0F;
    f32     top     = 320.0F;
    f32     znear    = 500.0F;
    f32     zfar     = 2000.0F;
    
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
    u32 i;
    f32 a;
	GXColor c = {255, 255, 255, 255};

	GXSetCullMode(GX_CULL_NONE);

    // for generated models
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
    GXSetVtxDesc(GX_VA_POS, GX_INDEX8);
	GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

    GXSetArray(GX_VA_POS, MyVert, 3*sizeof(f32));
	GXSetArray(GX_VA_NRM, MyVert, 3*sizeof(f32));

    // 
    // set up light parameters
    //

	GXInitLightPos(&MyLight, 0.0F, 0.0F, 0.0F);
    GXInitLightColor(&MyLight, c);
    GXLoadLightObjImm(&MyLight, GX_LIGHT0);

	c.g = c.b = 0;
	GXSetChanMatColor(GX_COLOR0, c);
    
    GXSetNumChans( 1 );
    GXSetNumTexGens( 0 );
    GXSetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);

    a = 0.0F;
    for (i = 0; i < 16; i++)
    {
        a = i * 2.0F * PI / 16.0F;
        MyVert[3*i] = sinf(a);   // x
        MyVert[3*i+1] = cosf(a); // y
		MyVert[3*i+2] = 0.0F;	// z
    }

	MyVert[3*16] = 0;   // x
    MyVert[3*16+1] = 0; // y
	MyVert[3*16+2] = 1;	// z

	MyVert[3*17] = 0;   // x
    MyVert[3*17+1] = 0; // y
	MyVert[3*17+2] = -1;// z

    // flush  array from CPU $
	DCFlushRange(MyVert, 54 * sizeof(f32));
}


/*---------------------------------------------------------------------------*
    Name:           DrawTick
    
    Description:    Draw the model once. 
                    GXInit makes GX_PNMTX0 the default matrix.
                    
    Arguments:      v        view matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick( Mtx v )
{
    u8  i, j, k;
    Mtx  ms;  // Model matrix. scale
    Mtx  mry, mrx;  // Model matrix. rotate
    Mtx  mt;  // Model matrix. translate
    Mtx  mv;  // Modelview matrix.
	float scale;


	GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,  // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_VTX,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);
   
    // build each segment matrix
    // assumes less than 10 segs for now...
	for (k = 0; k < 6; k++)
    {
    	for (i = 0; i < 10; i++)
    	{
			scale = ComputeScale(segs[(i + (10 * k)) - k].xlate);
    	    MTXScale(ms, scale, scale, 100.0F);
			MTXTrans(mt, 0.0F, 0.0F, segs[(i + (10 * k)) - k].xlate);
			MTXRotDeg(mry, 'Y', rot);
			MTXRotDeg(mrx, 'X', rot2 * (segs[(i + (10 * k)) - k].xlate/800.0F));

			MTXConcat(mt, ms, mv); 
			MTXConcat(mrx, mv, mv); 
			MTXConcat(mry, mv, mv); 
    	    MTXConcat(v, mv, mv); 
    	    GXLoadPosMtxImm(mv, segs[(i + (10 * k)) - k].mtx);

			if(scale == 100.00F) // segment is not in the bulge, do not adjust normals
			{
				MTXInverse(mv, mv);
				MTXTranspose(mv, mv);
				segs[(i + (10 * k)) - k].recomputeNormalMtx = 0;
				GXLoadNrmMtxImm(mv, segs[(i + (10 * k)) - k].mtx);
			}
			else
				segs[(i + (10 * k)) - k].recomputeNormalMtx = scale;	
    	}
	
    	// draw each segment
		// assume number of segs less than matrix memory size for now
		for (j = 0; j < 9; j++)
   		{

			for (i = 0; i < 16; i++) 
	        {
				if(segs[((j + (10 * k)) - k) + 1].recomputeNormalMtx)
					ComputeNormalRotMtx(&(segs[((j + (10 * k)) - k) + 1]), i);
				if(segs[((j + (10 * k)) - k)].recomputeNormalMtx)
					ComputeNormalRotMtx(&(segs[((j + (10 * k)) - k)]), i);
	        }
	        // finish cylinder
			if(segs[((j + (10 * k)) - k) + 1].recomputeNormalMtx)
				ComputeNormalRotMtx(&(segs[((j + (10 * k)) - k) + 1]), 0);
			if(segs[((j + (10 * k)) - k)].recomputeNormalMtx)
				ComputeNormalRotMtx(&(segs[((j + (10 * k)) - k)]), 0);

			// make this a function of circle divisions
	        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT3, 17*2);
	        for (i = 0; i < 16; i++) 
	        {
	            GXMatrixIndex1u8(segs[((j + (10 * k)) - k) + 1].mtx);
	            GXPosition1x8(i);
				GXNormal1x8(i);
				GXColor1u32(MyColors[k]);

	            GXMatrixIndex1u8(segs[((j + (10 * k)) - k)].mtx);
	            GXPosition1x8(i);
				GXNormal1x8(i);
				GXColor1u32(MyColors[k]);
	        }
	            // finish cylinder
	            GXMatrixIndex1u8(segs[((j + (10 * k)) - k) + 1].mtx);
	            GXPosition1x8(0);
				GXNormal1x8(0);
				GXColor1u32(MyColors[k]);

	            GXMatrixIndex1u8(segs[((j + (10 * k)) - k)].mtx);
	            GXPosition1x8(0);
				GXNormal1x8(0);
				GXColor1u32(MyColors[k]);
	        GXEnd();
	    }
	}

	GXSetChanCtrl(
        GX_COLOR0,
        GX_ENABLE,  // enable channel
        GX_SRC_REG,  // amb source
        GX_SRC_REG,  // mat source
        GX_LIGHT0,   // light mask
        GX_DF_CLAMP, // diffuse function
        GX_AF_NONE);

	MTXTrans(mt, 0.0F, 0.0F, BallPosition);
	MTXRotDeg(mry, 'Y', rot);
	MTXScale(ms, 125.0F, 125.0F, 125.0F);
	MTXRotDeg(mrx, 'X', rot2 * (BallPosition/800.0F));

	MTXConcat(mt, ms, mv); 
	MTXConcat(mrx, mv, mv); 
	MTXConcat(mry, mv, mv); 
    MTXConcat(v, mv, mv); 
    GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);

    GXSetCurrentMtx(GX_PNMTX0);

	GXDrawSphere1(2);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    Changes scene parameters.
                    
    Arguments:      none
    
    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick( void )
{
    if (DEMOPadGetStickX(0) > 0)
        rot += 2;
    else if (DEMOPadGetStickX(0) < 0)
        rot -= 2;

	if(direction)
	{
		 rot2 --;
		 if(rot2 < -30)
			direction = 0;
	}
	else
	{
		rot2 ++;
		if(rot2 > 30)
			direction = 1;
	}

	BallPosition += BallStep;
	if(BallPosition < -800) BallStep = 10;
	if(BallPosition > 800) BallStep = -10;
}

/*---------------------------------------------------------------------------*/
static float ComputeScale ( float position )
{
	float distance;
	float scale;

	distance = BallPosition - position;

	if(distance < 0) distance = -distance;

	scale = (150 * 150) - (distance * distance);

	if(scale < 0.0F)
		return 100.0F;

	scale = sqrtf(scale);

	if(scale < 100.0F)
		return 100.0F;

	return scale;
}

/*---------------------------------------------------------------------------*/
static void ComputeNormalRotMtx ( Segment *seg, u32 idx )
{
	Mtx mv, mra, mry, mrx;
	float scale = seg->recomputeNormalMtx;
	Vec axis;
	Vec normal = ((VecPtr)MyVert)[idx];

	if(seg->xlate < BallPosition)
		axis = ((VecPtr)MyVert)[16];
	else
		axis = ((VecPtr)MyVert)[17];

	VECCrossProduct(&axis, &normal, &axis);

	MTXRotAxisDeg(mra, &axis, ((150.0F - scale) / 50.0F) * 70.0F);

	MTXRotDeg(mry, 'Y', rot);
	MTXRotDeg(mrx, 'X', rot2 * (seg->xlate/800.0F));

	MTXConcat(mrx, mra, mv); 
	MTXConcat(mrx, mv, mv); 
	MTXConcat(mry, mv, mv); 
    MTXConcat(v, mv, mv); 

	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);

	GXLoadNrmMtxImm(mv, seg->mtx);
}

/*===========================================================================*/