/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     tg-clr-persp.c

  Copyright 1998 - 2001 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/TexGen/tg-clr-persp.c $
    
    3     7/23/01 9:04p Hirose
    Fixed MTXRotAxis -> MTXRotAxisDeg.
    
    2     7/13/01 5:58p Hirose
    Fixed a GXVerify warning message.
    
    1     3/31/01 6:05p Dante
    Initial Check-in
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

/*---------------------------------------------------------------------------*
  Global Data
 *---------------------------------------------------------------------------*/
// view matrix
Mtx g_viewMtx; 
// Draw mode
u8 g_mode = 0;
// What texture maps to use
u8 g_map = 0;
// Base texture?
u8 g_base = 0;  
// Alpha texture?
u8 g_alpha = 1;  
// Vectors to keep track of the camera's coordinate system orientation
Vec g_camX = {1.0F, 0.0F, 0.0F};  
Vec g_camY = {0.0F, 1.0F, 0.0F};
Vec g_camZ = {0.0F, 0.0F, 1.0F};
// Distance (Z) of Cam (one for each mode)
f32 g_dist[3] = {800.0F, 800.0F, 800.0F};
#define MAX_DIST 2500.0F
#define MIN_DIST 10.0F
// Texture maps
#define NUM_TEXTURES 15
GXTexObj g_texMap[NUM_TEXTURES];

/*---------------------------------------------------------------------------*
  Macros
 *---------------------------------------------------------------------------*/
#define COLOR_ADJUST(color, size) \
	(u8)(((size) / (size - 1)) * color + (((size) / (size - 1)) * 128))

/*---------------------------------------------------------------------------*
  CubeModelData
 *---------------------------------------------------------------------------*/
// Vertices
static f32 cube_vrt_f32[] ATTRIBUTE_ALIGN(32) = {
	-128.0F,  128.0F,  128.0F,
	128.0F,  128.0F,  128.0F,
	128.0F, -128.0F,  128.0F,
	-128.0F, -128.0F,  128.0F,
	-128.0F,  128.0F, -128.0F,
	128.0F,  128.0F, -128.0F,
	128.0F, -128.0F, -128.0F,
	-128.0F, -128.0F, -128.0F,
};
// Colors
static u32 cube_clr_u32[] ATTRIBUTE_ALIGN(32) = {
	0xff0000ff,
	0x00ff00ff,
	0x0000ffff,
	0xff00ffff,
	0xff000010,
	0x00ff0010,
	0x0000ff10,
	0xffffff10,
};
// Colors (RG)
static u32 cube_clr0_u32[] ATTRIBUTE_ALIGN(32) = {
	0xff000000,
	0x00ff0000,
	0x00000000,
	0xff000000,
	0xff000000,
	0x00ff0000,
	0x00000000,
	0xffff0000,
};
// Colors (BA) Channels moved into (RG) Channels
static u32 cube_clr1_u32[] ATTRIBUTE_ALIGN(32) = {
	0x00ff0000,
	0x00ff0000,
	0xffff0000,
	0xffff0000,
	0x00100000,
	0x00100000,
	0xff100000,
	0xff100000,
};
// Texture 
static f32 cube_txt_f32[] ATTRIBUTE_ALIGN(32) = 
{       
	0.0F, 0.0F,
	1.0F, 0.0F,
	1.0F, 1.0F,
	0.0F, 1.0F,
};
// Face Info
static u32 cube_quad_order[6][4] = {
	// Front
	{0, 1, 2, 3},
	// Right
	{1, 5, 6, 2},
	// Back
	{5, 4, 7, 6},
	// Left
	{4, 0, 3, 7},
	// Top
	{4, 5, 1, 0},
	// Bottom
	{3, 2, 6, 7},
};
static u32 cube_tri_order[12][3] = {
	// Front
	{0, 1, 3}, {1, 2, 3},
	// Right
	{1, 5, 2}, {5, 6, 2},
	// Back
	{5, 4, 6}, {4, 7, 6},
	// Left
	{4, 0, 7}, {0, 3, 7},
	// Top
	{4, 5, 0}, {5, 1, 0},
	// Bottom
	{3, 2, 7}, {2, 6, 7},
};
static u32 cube_tex_order[12] = {0, 1, 3, 1, 2, 3, 0};

/*---------------------------------------------------------------------------*
  WallData
 *---------------------------------------------------------------------------*/
#define WALL_SIZE 64

/*---------------------------------------------------------------------------*
  LightData
 *---------------------------------------------------------------------------*/
// Number of Lights
u8 g_numHwLights = 4;
GXLightObj g_lightObj[8];
// Vertices
static Vec light_vtx_vec[] ATTRIBUTE_ALIGN(32) = {
	{-128.0F, 128.0F, 30.0F},
	{-128.0F, -128.0F, 30.0F},
	{128.0F, 128.0F, 30.0F},
	{128.0F, -128.0F, 30.0F},
};
// Colors
static GXColor light_clr_clr[] ATTRIBUTE_ALIGN(32) = {
	{0xc0, 0x00, 0x00, 0xff},  // burgandy
	{0x70, 0xc0, 0x40, 0xff},  // olive green
	{0xc0, 0xa0, 0x70, 0xff},  // beige
	{0x40, 0x00, 0xc0, 0xff},  // indigo
};
GXColor g_ambClr = {0x00, 0x00, 0x00, 0x00};
GXColor g_matClr = {0xFF, 0xFF, 0xFF, 0xFF};

/*---------------------------------------------------------------------------*
  PlaneData
 *---------------------------------------------------------------------------*/
// Colors
static u32 plane_clr0_u32[] ATTRIBUTE_ALIGN(32) = {
	0xff0000ff,
	0x00ff00ff,
	0x0000ffff,
	0xff00ffff,
};
// Colors (BA)
static u32 plane_clr1_u32[] ATTRIBUTE_ALIGN(32) = {
	0x00ff0000,
	0x00ff0000,
	0xffff0000,
	0xffff0000,
};

/*---------------------------------------------------------------------------*
   Forward references
 *---------------------------------------------------------------------------*/
static void CameraInit(Mtx v);
static void MakeModelMtx(Vec xAxis, Vec yAxis, Vec zAxis, Mtx m);
static void DrawCube(u32 * cube_clr0_u32, u32 * cube_clr1_u32, 
		f32 * cube_tex_f32);
static void AnimTick(u8 * mode);
static void DrawTick(u8 mode);
static void DrawInit(void);
static void DrawState(u8 mode);
static void DrawWall(u16 size);
static void DrawZPlane(u32 * wall_clr0_u32, u32 * wall_clr1_u32);

/*---------------------------------------------------------------------------*
   Application main loop
 *---------------------------------------------------------------------------*/
void main ( void )
{
	DEMOInit(NULL);	   // Init os, pad, gx, vi

	CameraInit(g_viewMtx);

	DrawInit();

	OSReport("********************************\n");
	while (1)
	{
		AnimTick(&g_mode);
		DEMOBeforeRender();
		DrawTick(g_mode);
		DEMODoneRender();
		DEMOPadRead();
	}
	OSHalt("End of demo");
}

/*---------------------------------------------------------------------------*
   Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
	Name:           CameraInit
	
	Description:    Initialize the projection matrix and load into hardware.
					Initialize the view matrix
					
	Arguments:      v    view matrix
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit ( Mtx v )
{
	Mtx44 p;
	Vec   camPt = {0.0F, 0.0F, 800.0F};
	Vec   at    = {0.0F, 0.0F, -100.0F};
	Vec   up    = {0.0F, 1.0F, 0.0F};

	MTXFrustum(p, 240.0F,-240.0F,-320.0F, 320.0F, 400, 2000);
	MTXPerspective(p, 90.0F, 4.0F/3.0F, 5.0F, 10000);
	GXSetProjection(p, GX_PERSPECTIVE);
	MTXLookAt(v, &camPt, &up, &at);        
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
	Name:           DrawCube
	
	Description:    Draws a Cube.
					
	Arguments:      
					cube_clr0_u32  if not Null the color0 vtx data
								   (must be an array of 8 u32s)
					cube_clr1_u32  if not Null the color1 vtx data
								   (must be an array of 8 u32s)
					cube_tex_f32   if not NULL the texture coords
								   (must be an array of 4 f32s);
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawCube(u32 * cube_clr0_u32, u32 * cube_clr1_u32, 
		f32 * cube_tex_f32)
{
	// Face Info
	u32 i, j;

	// Setup
	GXClearVtxDesc();

	// Vertices
	GXSetVtxDesc(GX_VA_POS,  GX_INDEX8);
	GXSetArray(GX_VA_POS,  cube_vrt_f32, 3*sizeof(f32));
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_F32, 0);

	// Colors
	if (cube_clr0_u32)
	{
		GXSetVtxDesc(GX_VA_CLR0, GX_INDEX8);
		GXSetArray(GX_VA_CLR0, cube_clr0_u32, 1*sizeof(u32));
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	}

	// Colors
	if (cube_clr1_u32)
	{
		GXSetVtxDesc(GX_VA_CLR1, GX_INDEX8);
		GXSetArray(GX_VA_CLR1, cube_clr1_u32, 1*sizeof(u32));
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);
	}

	// Textures
	if (cube_txt_f32)
	{
		GXSetVtxDesc(GX_VA_TEX0, GX_INDEX8);
		GXSetArray(GX_VA_TEX0, cube_txt_f32, 2*sizeof(f32));
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	}

	// Draw a cube
	for (i = 0; i < 6; i++)
	{
		GXBegin(GX_QUADS, GX_VTXFMT0, 4);
		for (j = 0; j < 4; j++)
		{
			GXPosition1x8((u8)cube_quad_order[i][j]);
			if (cube_clr0_u32) GXColor1x8((u8)cube_quad_order[i][j]);
			if (cube_clr1_u32) GXColor1x8((u8)cube_quad_order[i][j]);
			if (cube_tex_f32) GXTexCoord1x8((u8)j);
		}
		GXEnd();
	}
}

/*---------------------------------------------------------------------------*
	Name:           DrawWall
	
	Description:    Draws a tesselated wall
					
	Arguments:      size of the wall. The wall will be sizeXsize
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawWall(u16 size)
{
	s32 x, y, i;
	f32 xx, yy0, yy1;
	f32 half;
#define STEP 10

	half = size / 2;

	// Setup
	GXClearVtxDesc();

	// Vertices
	GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

	// Normal
	GXSetVtxDesc(GX_VA_NRM,  GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);

	for (y = 0; y < size - 1; ++y)
	{
		i = y * size;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, (u16)(size * 2));
		for (x = 0; x < size; ++x)
		{
			xx = (f32)(x * STEP - half * STEP);
			yy0 = (f32)(y * STEP - half * STEP);
			yy1 = (f32)((y + 1) * STEP - half * STEP);
			GXPosition3f32(xx, yy0, 0.0F);
			GXNormal3f32(0.0F, 0.0F, 1.0F);
			GXPosition3f32(xx, yy1, 0.0F);
			GXNormal3f32(0.0F, 0.0F, 1.0F);
		}
		GXEnd();
	}
}

/*---------------------------------------------------------------------------*
	Name:           DrawZPlane
	
	Description:    Draws a quad in the X-Y axis
					
	Arguments:      
					cube_clr0_u32  the color0 vtx data (will be written)
								   (must be an array of 4 u32s)
					cube_clr1_u32  the color1 vtx data (will be written)
								   (must be an array of 4 u32s)
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawZPlane(u32 * wall_clr0_u32, u32 * wall_clr1_u32)
{

	// Setup
	GXClearVtxDesc();

	// Vertices
	GXSetVtxDesc(GX_VA_POS,  GX_DIRECT);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

	// Colors
	if (wall_clr0_u32)
	{
		GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
		GXSetArray(GX_VA_CLR0, wall_clr0_u32, 1*sizeof(u32));
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	}

	// Colors
	if (wall_clr1_u32)
	{
		GXSetVtxDesc(GX_VA_CLR1, GX_INDEX16);
		GXSetArray(GX_VA_CLR1, wall_clr1_u32, 1*sizeof(u32));
		GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_CLR_RGBA, GX_RGBA8, 0);
	}

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
	GXPosition3f32(0.0F, -64.0F,  1500.0F);
	if (wall_clr0_u32) GXColor1x16(0);
	if (wall_clr1_u32) GXColor1x16(0);
	GXPosition3f32(0.0F,  64.0F,  1500.0F);
	if (wall_clr0_u32) GXColor1x16(1);
	if (wall_clr1_u32) GXColor1x16(1);
	GXPosition3f32(0.0F, -64.0F, -1500.0F);
	if (wall_clr0_u32) GXColor1x16(2);
	if (wall_clr1_u32) GXColor1x16(2);
	GXPosition3f32(0.0F,  64.0F, -1500.0F);
	if (wall_clr0_u32) GXColor1x16(3);
	if (wall_clr1_u32) GXColor1x16(3);
	GXEnd();
}

/*---------------------------------------------------------------------------*
	Name:           DrawTick
	
	Description:    
					
	Arguments:      mode   current draw mode
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawTick(u8 mode)
{
	switch (mode)
	{
		case 0:
			if (!g_base)
			{
				DrawState(0);
				GXSetCurrentMtx(GX_PNMTX0);
				// Note: Drawing twice for cheap z-sort
				GXSetCullMode(GX_CULL_FRONT);
				DrawCube(cube_clr_u32, NULL, cube_txt_f32);
				GXSetCullMode(GX_CULL_BACK);
				DrawCube(cube_clr_u32, NULL, cube_txt_f32);

				DrawState(1);
				GXSetCurrentMtx(GX_PNMTX1);
				// Note: Drawing twice for cheap z-sort
				GXSetCullMode(GX_CULL_FRONT);
				DrawCube(cube_clr0_u32, cube_clr1_u32, cube_txt_f32);
				GXSetCullMode(GX_CULL_BACK);
				DrawCube(cube_clr0_u32, cube_clr1_u32, cube_txt_f32);
			}
			else
			{
				DrawState(2);
				GXSetCurrentMtx(GX_PNMTX0);
				// Note: Drawing twice for cheap z-sort
				GXSetCullMode(GX_CULL_FRONT);
				DrawCube(cube_clr_u32, NULL, cube_txt_f32);
				GXSetCullMode(GX_CULL_BACK);
				DrawCube(cube_clr_u32, NULL, cube_txt_f32);

				DrawState(3);
				GXSetCurrentMtx(GX_PNMTX1);
				// Note: Drawing twice for cheap z-sort
				GXSetCullMode(GX_CULL_FRONT);
				DrawCube(cube_clr0_u32, cube_clr1_u32, cube_txt_f32);
				GXSetCullMode(GX_CULL_BACK);
				DrawCube(cube_clr0_u32, cube_clr1_u32, cube_txt_f32);
			}
			break;
		case 1:
			GXSetCurrentMtx(GX_PNMTX7);
			DrawState(4);
			GXSetCullMode(GX_CULL_NONE);
			DrawWall(WALL_SIZE);
			break;
		case 2:
			GXSetCullMode(GX_CULL_NONE);

			DrawState(0);
			GXSetCurrentMtx(GX_PNMTX0);
			DrawZPlane(plane_clr0_u32, NULL);

			DrawState(1);
			GXSetCurrentMtx(GX_PNMTX1);
			DrawZPlane(plane_clr0_u32, cube_clr1_u32);
			break;
	}
}

/*---------------------------------------------------------------------------*
	Name:           AnimTick
	
	Description:    
					
	Arguments:      mode   pointer to current draw mode
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimTick(u8 * mode)
{
	static f32 rotX = 0, rotY = 0;
	static f32 lrotZ = 0;
	static u8 rotate = 1;
	s8 stickX, stickY;
	s8 substickX, substickY;
	Mtx temp;
	Mtx mtx0;
	Mtx mtx1;
	f32 ddist;
	u16 button;
	u16 i;
	u16 w, h;

	button = DEMOPadGetButtonDown(0);
	stickX = DEMOPadGetStickX(0);
	stickY = DEMOPadGetStickY(0);
	substickX = DEMOPadGetSubStickX(0);
	substickY = DEMOPadGetSubStickY(0);

	// Global Button Controls
	switch (button)
	{
		// Swap Texture!
		case PAD_BUTTON_X:
			g_map = (u8)((g_map + 1) % 7);
			GXLoadTexObj(&g_texMap[2 * g_map], GX_TEXMAP0);
			GXLoadTexObj(&g_texMap[2 * g_map + 1], GX_TEXMAP1);
			w = GXGetTexObjWidth(&g_texMap[2 * g_map]);
			h = GXGetTexObjHeight(&g_texMap[2 * g_map]);
			OSReport("Rasterization Lookup Texture: %d (%dx%d)\n", 
					g_map, w, h);
			break;
		case PAD_BUTTON_A:
			*mode = (u8)((*mode + 1) % 3);
			break;
		case PAD_BUTTON_B:
			rotate ^= 1;
			break;
	}

	// Mode Controls
	switch (*mode)
	{
		case 0:
			// Cube Rotation
			if (rotate)
			{
				rotX += 0.33F;
				rotY += 0.32F;
			}
			rotX += (f32)stickX / 3.0F;
			rotY -= (f32)stickY / 3.0F;

			// Button Controls
			switch (button)
			{
				case PAD_BUTTON_UP:
					g_base ^= 1;
					break;
				case PAD_BUTTON_LEFT:
					g_alpha ^= 1;
					break;
			}

			// Stick Controls
			if (substickX || substickY)
			{
				if (substickX)
				{
					if (stickX > 0)
						MTXRotAxisDeg(temp, &g_camY, 3.0F);
					else
						MTXRotAxisDeg(temp, &g_camY, -3.0F);

					MTXMultVec(temp, &g_camX, &g_camX);
					MTXMultVec(temp, &g_camZ, &g_camZ);
				}

				if (substickY)
				{
					if (stickY > 0)
						MTXRotAxisDeg(temp, &g_camX, 3.0F);
					else
						MTXRotAxisDeg(temp, &g_camX, -3.0F);

					MTXMultVec(temp, &g_camY, &g_camY);
					MTXMultVec(temp, &g_camZ, &g_camZ);
				}
			}
			ddist = DEMOPadGetTriggerL(0) / 3.0F;
			if (g_dist[0] - ddist > MIN_DIST) g_dist[0] -= ddist;
			ddist = DEMOPadGetTriggerR(0) / 3.0F;
			if (g_dist[0] + ddist < MAX_DIST) g_dist[0] += ddist;

			// Create View Matrix
			MakeModelMtx(g_camX, g_camY, g_camZ, g_viewMtx);
			MTXTranspose(g_viewMtx, g_viewMtx);
			MTXTrans(temp, 0.0F, 0.0F, -g_dist[0]);
			MTXConcat(temp, g_viewMtx, g_viewMtx);

			// Cube 0
			MTXTrans(mtx0, -200.0F, 0.0F, 0.0F);
			MTXRotDeg(temp, 'x', rotX);
			MTXConcat(mtx0, temp, mtx0);
			MTXRotDeg(temp, 'y', rotY);
			MTXConcat(mtx0, temp, mtx0);
			MTXConcat(g_viewMtx, mtx0, mtx0);

			// Cube 1
			MTXTrans(mtx1, 200.0F, 0.0F, 0.0F);
			MTXRotDeg(temp, 'x', rotX);
			MTXConcat(mtx1, temp, mtx1);
			MTXRotDeg(temp, 'y', rotY);
			MTXConcat(mtx1, temp, mtx1);
			MTXConcat(g_viewMtx, mtx1, mtx1);

			GXLoadPosMtxImm(mtx0, GX_PNMTX0);
			GXLoadPosMtxImm(mtx1, GX_PNMTX1);
			break;
		case 1:
			// Camera Control
			ddist = (f32)substickY / 5.0F;
			if (g_dist[1] - ddist < MAX_DIST && g_dist[1] - ddist > MIN_DIST)
			{
				g_dist[1] -= ddist;
			}
			MTXIdentity(g_viewMtx);
			MTXTrans(temp, 0.0F, 0.0F, -g_dist[1]);
			MTXConcat(temp, g_viewMtx, g_viewMtx);

			// Model Control
			GXLoadPosMtxImm(g_viewMtx, GX_PNMTX7);

			// Normal
			MTXInverse(g_viewMtx, temp); 
			MTXTranspose(temp, temp); 
			GXLoadNrmMtxImm(temp, GX_PNMTX7);

			// Rotate Lights
			if (rotate)
			{
				lrotZ = 0.32F;
			}
			else
			{
				lrotZ = 0.0F;
			}
			MTXRotDeg(temp, 'z', lrotZ);
			for (i = 0; i < g_numHwLights; ++i)
			{
				MTXMultVec(temp, &light_vtx_vec[i], &light_vtx_vec[i]);
			}

			break;
		case 2:
			// Camera Control
			ddist = (f32)substickY / 5.0F;
			if (g_dist[2] - ddist < MAX_DIST && g_dist[2] - ddist > MIN_DIST)
			{
				g_dist[2] -= ddist;
			}
			MTXIdentity(g_viewMtx);
			MTXTrans(temp, 0.0F, 0.0F, -g_dist[2]);
			MTXConcat(temp, g_viewMtx, g_viewMtx);

			// Model Control
			MTXTrans(mtx0, -200.0F, 0.0F, 0.0F);
			MTXConcat(g_viewMtx, mtx0, mtx0);
			MTXTrans(mtx1, 200.0F, 0.0F, 0.0F);
			MTXConcat(g_viewMtx, mtx1, mtx1);
			GXLoadPosMtxImm(mtx0, GX_PNMTX0);
			GXLoadPosMtxImm(mtx1, GX_PNMTX1);
			break;
	}
}

/*---------------------------------------------------------------------------*
	Name:           DrawInit
	
	Description:    Initializes Drawing state
					
	Arguments:      none
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawInit(void)
{
	TEXDescriptorPtr tdp;
	TEXPalettePtr tpl = 0;
	GXColor black = {0, 0, 0, 0};
	u32 i;

	// Setup Textures
	TEXGetPalette(&tpl, "gxTests/tg-pc.tpl");
	// First
	for (i = 0; i < NUM_TEXTURES; i++)
	{
		tdp = TEXGet(tpl, i);
		GXInitTexObj(&g_texMap[i], 
				tdp->textureHeader->data, 
				tdp->textureHeader->width, 
				tdp->textureHeader->height, 
				(GXTexFmt)tdp->textureHeader->format,
				GX_CLAMP, 
				GX_CLAMP, 
				GX_FALSE);
	}
	GXLoadTexObj(&g_texMap[0], GX_TEXMAP0);
	GXLoadTexObj(&g_texMap[1], GX_TEXMAP1);
	GXLoadTexObj(&g_texMap[NUM_TEXTURES - 1], GX_TEXMAP2);

	// Global Modes
	GXSetCullMode(GX_CULL_NONE);

	// Copy 
	GXSetDispCopyGamma(GX_GM_1_0);
	GXSetCopyClear(black, GX_MAX_Z24);
}

/*---------------------------------------------------------------------------*
	Name:           DrawState
	
	Description:    Set's draw state
					
	Arguments:      mode 	What mode to draw
	
	Returns:        none
 *---------------------------------------------------------------------------*/
static void DrawState(u8 mode)
{
	u8 i;
	Vec temp;
	GXColor color;
	u32 mask0 = 0;
	u32 mask1 = 0;
	int lookup[] = {
		GX_LIGHT0, GX_LIGHT1, GX_LIGHT2, GX_LIGHT3, 
		GX_LIGHT4, GX_LIGHT5, GX_LIGHT6, GX_LIGHT7};

	if (g_alpha)
	{
		GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
	}
	else
	{
		GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
	}

	switch (mode)
	{
		case 0:
			/*-------------------------------------------------*
			  Rasterize Color State (vtx color => final color)
			  => Not Perspective Correct Interpolation
			 *-------------------------------------------------*/

			// Don't forget these!
			GXSetNumChans(1);
			GXSetNumTexGens(0);
			GXSetNumTevStages(1);

			// Color Channel (Vertex Color)
			GXSetChanCtrl(GX_COLOR0A0,
					GX_DISABLE,
					GX_SRC_VTX,
					GX_SRC_VTX,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);

			// Tev 0
			GXSetTevOrder(GX_TEVSTAGE0,
					GX_TEXCOORD_NULL,
					GX_TEXMAP_NULL,
					GX_COLOR0A0);
			GXSetTevColorIn(GX_TEVSTAGE0,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_RASC);
			GXSetTevAlphaIn(GX_TEVSTAGE0,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_RASA);
			GXSetTevColorOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);

			break;
		case 1:
			/*-------------------------------------------------*
			  Rasterize Color => Texture Coords => Perspective
				 Correct Texture Interpolation
			 *-------------------------------------------------*/

			// Don't forget these!
			GXSetNumChans(2);
			GXSetNumTexGens(2);
			GXSetNumTevStages(2);

			// Color Channels
			// Send vertex color data through the color channels
			GXSetChanCtrl(GX_COLOR0A0,
					GX_DISABLE,
					GX_SRC_VTX,
					GX_SRC_VTX,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);
			GXSetChanCtrl(GX_COLOR1A1,
					GX_DISABLE,
					GX_SRC_VTX,
					GX_SRC_VTX,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);

			// Convert color0 to s,t for 2-D texture lookup (RG)
			GXSetTexCoordGen(GX_TEXCOORD0,
					GX_TG_SRTG,
					GX_TG_COLOR0,
					GX_IDENTITY);
			// Convert color1 to s,t for 2-D texture lookup (BA)
			GXSetTexCoordGen(GX_TEXCOORD1,
					GX_TG_SRTG,
					GX_TG_COLOR1,
					GX_IDENTITY);

			// Tev 0: Use rasterization lookup table (RG)
			// Note: Blue = 0 and Alpha = 0;
			GXSetTevOrder(GX_TEVSTAGE0,
					GX_TEXCOORD0,
					GX_TEXMAP0,
					GX_COLOR_NULL);
			GXSetTevColorIn(GX_TEVSTAGE0,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_TEXC);
			GXSetTevAlphaIn(GX_TEVSTAGE0,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);

			// Tev 1: Use rasterization lookup table (BA),
			// then add previous tev (RG + BA)
			GXSetTevOrder(GX_TEVSTAGE1,
					GX_TEXCOORD1,
					GX_TEXMAP1,
					GX_COLOR_NULL);
			GXSetTevColorIn(GX_TEVSTAGE1,
					GX_CC_ZERO,
					GX_CC_TEXC,
					GX_CC_ONE,
					GX_CC_CPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1,
					GX_CA_ZERO,
					GX_CA_TEXA,
					GX_CA_ONE,
					GX_CA_APREV);
			GXSetTevColorOp(GX_TEVSTAGE1,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			break;
		case 2:
			/*-------------------------------------------------*
			  Rasterize Color State (vtx color => final color)
			  => Not Perspective Correct Interpolation
				 + Base Texture
			 *-------------------------------------------------*/

			// Don't forget these!
			GXSetNumChans(1);
			GXSetNumTexGens(1);
			GXSetNumTevStages(1);

			// Color Channel (Vertex Color)
			GXSetChanCtrl(GX_COLOR0A0,
					GX_DISABLE,
					GX_SRC_VTX,
					GX_SRC_VTX,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);

			// Base Texture Coordinates
			GXSetTexCoordGen(GX_TEXCOORD0,
					GX_TG_MTX2x4,
					GX_TG_TEX0,
					GX_IDENTITY);

			// Tev 0
			GXSetTevOrder(GX_TEVSTAGE0,
					GX_TEXCOORD0,
					GX_TEXMAP2,
					GX_COLOR0A0);
			GXSetTevColorIn(GX_TEVSTAGE0,
					GX_CC_ZERO,
					GX_CC_RASC,
					GX_CC_TEXC,
					GX_CC_ZERO);
			GXSetTevAlphaIn(GX_TEVSTAGE0,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_RASA);
			GXSetTevColorOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);

			break;
		case 3:
			/*-------------------------------------------------*
			  Rasterize Color => Texture Coords => Perspective
				 Correct Texture Interpolation
				 + Base Texture
			 *-------------------------------------------------*/

			// Don't forget these!
			GXSetNumChans(2);
			GXSetNumTexGens(3);
			GXSetNumTevStages(3);

			// Color Channels
			// Send vertex color data through the color channels
			GXSetChanCtrl(GX_COLOR0A0,
					GX_DISABLE,
					GX_SRC_VTX,
					GX_SRC_VTX,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);
			GXSetChanCtrl(GX_COLOR1A1,
					GX_DISABLE,
					GX_SRC_VTX,
					GX_SRC_VTX,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);

			// Note: The following TexCoordGen order is important. 
			// See "Functional Ordering Requirements" on man page.

			// Base Texture Coordinates
			GXSetTexCoordGen(GX_TEXCOORD0,
					GX_TG_MTX2x4,
					GX_TG_TEX0,
					GX_IDENTITY);
			// Convert color0 to s,t for 2-D texture lookup (RG)
			GXSetTexCoordGen(GX_TEXCOORD1,
					GX_TG_SRTG,
					GX_TG_COLOR0,
					GX_IDENTITY);
			// Convert color1 to s,t for 2-D texture lookup (BA)
			GXSetTexCoordGen(GX_TEXCOORD2,
					GX_TG_SRTG,
					GX_TG_COLOR1,
					GX_IDENTITY);

			// Tev 0: Use rasterization lookup table (RG)
			// Note: Blue = 0 and Alpha = 0;
			GXSetTevOrder(GX_TEVSTAGE0,
					GX_TEXCOORD1,
					GX_TEXMAP0,
					GX_COLOR_NULL);
			GXSetTevColorIn(GX_TEVSTAGE0,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_TEXC);
			GXSetTevAlphaIn(GX_TEVSTAGE0,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO);
			GXSetTevColorOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);

			// Tev 1: Use rasterization lookup table (BA),
			// then add previous tev (RG + BA)
			GXSetTevOrder(GX_TEVSTAGE1,
					GX_TEXCOORD2,
					GX_TEXMAP1,
					GX_COLOR_NULL);
			GXSetTevColorIn(GX_TEVSTAGE1,
					GX_CC_ZERO,
					GX_CC_TEXC,
					GX_CC_ONE,
					GX_CC_CPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1,
					GX_CA_ZERO,
					GX_CA_TEXA,
					GX_CA_ONE,
					GX_CA_APREV);
			GXSetTevColorOp(GX_TEVSTAGE1,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);

			// Tev 2: Modulate Base Texture
			// No alpha in base texture, so we just pass previous alpha
			GXSetTevOrder(GX_TEVSTAGE2,
					GX_TEXCOORD0,
					GX_TEXMAP2,
					GX_COLOR_NULL);
			GXSetTevColorIn(GX_TEVSTAGE2,
					GX_CC_ZERO,
					GX_CC_TEXC,
					GX_CC_CPREV,
					GX_CC_ZERO);
			GXSetTevAlphaIn(GX_TEVSTAGE2,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_APREV);
			GXSetTevColorOp(GX_TEVSTAGE2,
					GX_TEV_ADD,
					GX_TB_ZERO, 
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE2,
					GX_TEV_ADD,
					GX_TB_ZERO, 
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			break;
		case 4:
			/*-------------------------------------------------*
				Rasterize Color w/ Lighting => Texture Coords 
				=> Perspective Correct Texture interpolation
			 *-------------------------------------------------*/

			// Turn on lights. Note that 2 lights are needed for a full
			// RGBA light, unless you only need a monochromatic light
			for (i = 0 ; i < g_numHwLights ; ++i)
			{
				// Convert light position into view space
				MTXMultVec(g_viewMtx,
						&light_vtx_vec[i],
						&temp);

				// RG
				GXInitLightPos(&g_lightObj[2 * i],
						temp.x,
						temp.y,
						temp.z);
				GXInitLightColor(&g_lightObj[2 * i],
						light_clr_clr[i]);
				// BA
				GXInitLightPos(&g_lightObj[2 * i + 1],
						temp.x,
						temp.y,
						temp.z);
				color.r = light_clr_clr[i].b;
				color.g = light_clr_clr[i].a;
				color.b = 0;
				color.a = 0;
				GXInitLightColor(&g_lightObj[2 * i + 1],
						color);

				// Load light object into hardware
				GXLoadLightObjImm(&g_lightObj[2 * i],
						(GXLightID)lookup[2 * i]);
				GXLoadLightObjImm(&g_lightObj[2 * i + 1],
						(GXLightID)lookup[2 * i + 1]);

				// Compute Mask
				mask0 |= lookup[2 * i];
				mask1 |= lookup[2 * i + 1];
			}

			// Don't forget these!
			GXSetNumChans(2);
			GXSetNumTexGens(2);
			GXSetNumTevStages(2);

			// Color Channels
			GXSetChanCtrl(GX_COLOR0,
					GX_ENABLE,
					GX_SRC_REG,
					GX_SRC_REG,
					mask0,
					GX_DF_CLAMP,
					GX_AF_NONE);
			GXSetChanCtrl(GX_COLOR1,
					GX_ENABLE,
					GX_SRC_REG,
					GX_SRC_REG,
					mask1,
					GX_DF_CLAMP,
					GX_AF_NONE);
			// Since we only us RG in texgen, alpha can be turned off.
			// Actually, you have to ifyou want to use 4 full color lights
			// (8 hardware lights total)
			GXSetChanCtrl (GX_ALPHA0,
					GX_DISABLE,
					GX_SRC_REG,
					GX_SRC_REG,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);
			GXSetChanCtrl(GX_ALPHA1,
					GX_DISABLE,
					GX_SRC_REG,
					GX_SRC_REG,
					GX_LIGHT_NULL,
					GX_DF_NONE,
					GX_AF_NONE);

			// set up ambient color
			GXSetChanAmbColor(GX_COLOR0A0,
					g_ambClr);
			GXSetChanAmbColor(GX_COLOR1A1,
					g_ambClr);
			// set up material color
			GXSetChanMatColor(GX_COLOR0A0,
					g_matClr);
			GXSetChanMatColor(GX_COLOR1A1,
					g_matClr);

			// Tev 0: Use rasterization lookup table (RG)
			GXSetTevOrder(GX_TEVSTAGE0,
					GX_TEXCOORD_NULL,
					GX_TEXMAP_NULL,
					GX_COLOR0A0);
			GXSetTevColorIn(GX_TEVSTAGE0,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_ZERO,
					GX_CC_RASC);
			GXSetTevAlphaIn(GX_TEVSTAGE0,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_ZERO,
					GX_CA_RASA);
			GXSetTevColorOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE0,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);

			// Tev 1: Use rasterization lookup table (BA),

			// then add previous tev (RG + BA)
			GXSetTevOrder(GX_TEVSTAGE1,
					GX_TEXCOORD1,
					GX_TEXMAP1,
					GX_COLOR1A1);
			GXSetTevColorIn(GX_TEVSTAGE1,
					GX_CC_ZERO,
					GX_CC_TEXC,
					GX_CC_ONE,
					GX_CC_CPREV);
			GXSetTevAlphaIn(GX_TEVSTAGE1,
					GX_CA_ZERO,
					GX_CA_TEXA,
					GX_CA_ONE,
					GX_CA_APREV);
			GXSetTevColorOp(GX_TEVSTAGE1,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);
			GXSetTevAlphaOp(GX_TEVSTAGE1,
					GX_TEV_ADD,
					GX_TB_ZERO,
					GX_CS_SCALE_1,
					GX_ENABLE,
					GX_TEVPREV);

			// convert color0 to s,t for 2-D texture lookup (RG)
			GXSetTexCoordGen(GX_TEXCOORD0,
					GX_TG_SRTG,
					GX_TG_COLOR0,
					GX_IDENTITY);
			// convert color1 to s,t for 2-D texture lookup (BA)
			GXSetTexCoordGen(GX_TEXCOORD1,
					GX_TG_SRTG,
					GX_TG_COLOR1, 
					GX_IDENTITY);
			break;
	}
}

