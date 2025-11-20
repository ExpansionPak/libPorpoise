

/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     geo-fmtPrim.c

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Geometry/geo-vtx-fmtPrim.c $
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    5     11/03/99 7:22p Yasu
    Replace GX*1u8,u16 to GX*1x8,x16
    
    4     9/30/99 10:35p Yasu
    Renamed some GX functions and enums
    
    3     9/15/99 2:09p Ryan
    Update to fix compiler warnings
    
    2     8/13/99 11:48a Ryan
    
    1     8/11/99 11:04a Ryan
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>

void DrawQuadCube( void );
void DrawTriCube( void );
void DrawPointCube( void );
void DrawLineCube( void );
void DrawLineStripCube( void );
void DrawTriStripCube( void );
void DrawTriFanCube( void );


static void SendVertex ( u16 posIndex, u16 normalIndex, u16 colorIndex, u16 texCoordIndex );

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
extern Mtx v;

extern float 	FloatVert[];
extern s16 		Vert16[];
extern s8 		Vert8[];

extern float 	FloatNorm[];
extern s16 		Norm16[];
extern s8 		Norm8[];

extern u8	ColorRGBA8[];
extern u8	ColorRGBA6[];
extern u8	ColorRGBA4[];
extern u8	ColorRGBX8[];
extern u8	ColorRGB8[];
extern u8	ColorRGB565[];

extern float 	FloatTex[];
extern u16 		Tex16[];
extern u8 		Tex8[];

extern u32 rot;

extern u8	ColorControl;
extern u8	NormalControl;
extern u8	TexCoordControl;
extern u8	PositionControl;


/*---------------------------------------------------------------------------*
    Name:			DrawQuadCube
    
    Description:	Draws the cube as Quads  
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
void DrawQuadCube( void )
{
	Mtx mv, r1, r2, t;

	MTXTrans(t, 0, -200, 0);
	MTXRotDeg(r1, 'X', (float)rot);
	MTXRotDeg(r2, 'y', (float)rot/2.0F);

	MTXConcat(r2, r1, mv);
	MTXConcat(t, mv, mv);
	MTXConcat(v, mv, mv);
	GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);

	GXBegin(GX_QUADS, GX_VTXFMT0, 4*6);

		SendVertex(0, 0, 1, 0);
		SendVertex(1, 0, 2, 1);
		SendVertex(2, 0, 3, 2);
		SendVertex(3, 0, 4, 3);

		SendVertex(4, 1, 0, 0);
		SendVertex(5, 1, 0, 1);
		SendVertex(6, 1, 0, 2);
		SendVertex(7, 1, 0, 3);		

		SendVertex(2, 2, 0, 0);
		SendVertex(6, 2, 0, 1);
		SendVertex(5, 2, 0, 2);
		SendVertex(3, 2, 0, 3);
		
		SendVertex(1, 3, 0, 0);
		SendVertex(0, 3, 0, 1);
		SendVertex(4, 3, 0, 2);
		SendVertex(7, 3, 0, 3);   
        
		SendVertex(5, 4, 0, 0);
		SendVertex(4, 4, 0, 1);
		SendVertex(0, 4, 0, 2);
		SendVertex(3, 4, 0, 3);
		
		SendVertex(6, 5, 0, 0);
		SendVertex(2, 5, 0, 1);
		SendVertex(1, 5, 0, 2);
		SendVertex(7, 5, 0, 3);

	GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:			DrawTriCube
    
    Description:	Draws the cube as Triangles  
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
void DrawTriCube( void )
{
	Mtx mv, r1, r2, t;

	MTXTrans(t, -250, 0, 0);
	MTXRotDeg(r1, 'X', (float)rot);
	MTXRotDeg(r2, 'y', (float)rot/2.0F);

	MTXConcat(r2, r1, mv);
	MTXConcat(t, mv, mv);
	MTXConcat(v, mv, mv);
	GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);
	

	GXBegin(GX_TRIANGLES, GX_VTXFMT0, 6*6);

		SendVertex(0, 0, 1, 0);
		SendVertex(1, 0, 2, 1);
		SendVertex(2, 0, 3, 2);
		SendVertex(2, 0, 3, 2);
		SendVertex(3, 0, 4, 3);
		SendVertex(0, 0, 1, 0);

		SendVertex(4, 1, 0, 0);
		SendVertex(5, 1, 0, 1);
		SendVertex(6, 1, 0, 2);
		SendVertex(6, 1, 0, 2);
		SendVertex(7, 1, 0, 3);
		SendVertex(4, 1, 0, 0);

		SendVertex(2, 2, 0, 0);
		SendVertex(6, 2, 0, 1);
		SendVertex(5, 2, 0, 2);
		SendVertex(5, 2, 0, 2);
		SendVertex(3, 2, 0, 3);
		SendVertex(2, 2, 0, 0);
		
        SendVertex(1, 3, 0, 0);
		SendVertex(0, 3, 0, 1);
		SendVertex(4, 3, 0, 2);
		SendVertex(4, 3, 0, 2);
		SendVertex(7, 3, 0, 3);
		SendVertex(1, 3, 0, 0);       
        
		SendVertex(5, 4, 0, 0);
		SendVertex(4, 4, 0, 1);
		SendVertex(0, 4, 0, 2);
		SendVertex(0, 4, 0, 2);
		SendVertex(3, 4, 0, 3);
		SendVertex(5, 4, 0, 0);

		SendVertex(6, 5, 0, 0);
		SendVertex(2, 5, 0, 1);
		SendVertex(1, 5, 0, 2);
		SendVertex(1, 5, 0, 2);
		SendVertex(7, 5, 0, 3);
		SendVertex(6, 5, 0, 0);

	GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:			DrawPointCube
    
    Description:	Draws the cube as Triangles  
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
void DrawPointCube( void )
{
	Mtx mv, r1, r2, t;

	MTXTrans(t, -250, 200, 0);
	MTXRotDeg(r1, 'X', (float)rot);
	MTXRotDeg(r2, 'y', (float)rot/2.0F);

	MTXConcat(r2, r1, mv);
	MTXConcat(t, mv, mv);
	MTXConcat(v, mv, mv);
	GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);
	

	GXBegin(GX_POINTS, GX_VTXFMT0, 8);

		SendVertex(0, 0, 1, 0);
		SendVertex(1, 0, 2, 1);
		SendVertex(2, 0, 3, 2);
		SendVertex(3, 0, 4, 3);
		SendVertex(4, 1, 0, 0);
		SendVertex(5, 1, 0, 1);
		SendVertex(6, 1, 0, 2);
		SendVertex(7, 1, 0, 3);

	GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:			DrawLineCube
    
    Description:	Draws the cube as Triangles  
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
void DrawLineCube( void )
{
	Mtx mv, r1, r2, t;

	MTXTrans(t, 0, 200, 0);
	MTXRotDeg(r1, 'X', (float)rot);
	MTXRotDeg(r2, 'y', (float)rot/2.0F);

	MTXConcat(r2, r1, mv);
	MTXConcat(t, mv, mv);
	MTXConcat(v, mv, mv);
	GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);
	

	GXBegin(GX_LINES, GX_VTXFMT0, 24);

		SendVertex(0, 0, 1, 0);
		SendVertex(1, 0, 2, 1);

		SendVertex(1, 0, 2, 1);
		SendVertex(2, 0, 3, 2);

		SendVertex(2, 0, 3, 2);
		SendVertex(3, 0, 4, 3);

		SendVertex(3, 0, 4, 3);
		SendVertex(0, 0, 1, 0);

		SendVertex(4, 1, 0, 0);
		SendVertex(5, 1, 0, 1);

		SendVertex(5, 1, 0, 1);
		SendVertex(6, 1, 0, 2);

		SendVertex(6, 1, 0, 2);
		SendVertex(7, 1, 0, 3);

		SendVertex(7, 1, 0, 3);
		SendVertex(4, 1, 0, 0);

		SendVertex(0, 0, 0, 0);
		SendVertex(4, 1, 0, 0);

		SendVertex(1, 0, 0, 1);
		SendVertex(7, 1, 0, 3);

		SendVertex(2, 0, 0, 2);
		SendVertex(6, 1, 0, 2);

		SendVertex(3, 0, 0, 3);
		SendVertex(5, 1, 0, 1);
		
	GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:			DrawLineStripCube
    
    Description:	Draws the cube as Triangles  
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
void DrawLineStripCube( void )
{
	Mtx mv, r1, r2, t;

	MTXTrans(t, 250, 200, 0);
	MTXRotDeg(r1, 'X', (float)rot);
	MTXRotDeg(r2, 'y', (float)rot/2.0F);

	MTXConcat(r2, r1, mv);
	MTXConcat(t, mv, mv);
	MTXConcat(v, mv, mv);
	GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);
	
	GXBegin(GX_LINESTRIP, GX_VTXFMT0, 10);

		SendVertex(0, 0, 1, 0);
		SendVertex(1, 0, 2, 1);
		SendVertex(2, 0, 3, 2);
		SendVertex(3, 0, 4, 3);
		SendVertex(0, 0, 0, 0);
		SendVertex(4, 1, 0, 0);
		SendVertex(5, 1, 0, 1);
		SendVertex(6, 1, 0, 2);
		SendVertex(7, 1, 0, 3);
		SendVertex(4, 1, 0, 0);

	GXEnd();

	GXBegin(GX_LINESTRIP, GX_VTXFMT0, 2);

		SendVertex(3, 0, 0, 3);
		SendVertex(5, 1, 0, 1);

	GXEnd();

	GXBegin(GX_LINESTRIP, GX_VTXFMT0, 2);

		SendVertex(1, 0, 0, 1);
		SendVertex(7, 1, 0, 3);

	GXEnd();

	GXBegin(GX_LINESTRIP, GX_VTXFMT0, 2);

		SendVertex(2, 0, 0, 2);
		SendVertex(6, 1, 0, 2);

	GXEnd();
}

/*---------------------------------------------------------------------------*
    Name:			DrawTriStripCube
    
    Description:	Draws the cube as Triangles  
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
void DrawTriStripCube( void )
{
	Mtx mv, r1, r2, t;

	MTXTrans(t, 0, 0, 0);
	MTXRotDeg(r1, 'X', (float)rot);
	MTXRotDeg(r2, 'y', (float)rot/2.0F);

	MTXConcat(r2, r1, mv);
	MTXConcat(t, mv, mv);
	MTXConcat(v, mv, mv);
	GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);
	
	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

		SendVertex(1, 0, 1, 1);
		SendVertex(2, 0, 2, 2);
		SendVertex(0, 0, 3, 0);
		SendVertex(3, 0, 4, 3);

	GXEnd();

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

		SendVertex(5, 1, 0, 1);
		SendVertex(6, 1, 0, 2);
		SendVertex(4, 1, 0, 0);
		SendVertex(7, 1, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

		SendVertex(6, 2, 0, 1);
		SendVertex(5, 2, 0, 2);
		SendVertex(2, 2, 0, 0);
		SendVertex(3, 2, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

		SendVertex(0, 3, 0, 1);
		SendVertex(4, 3, 0, 2);
		SendVertex(1, 3, 0, 0);
		SendVertex(7, 3, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);   

		SendVertex(4, 4, 0, 1);
		SendVertex(0, 4, 0, 2);
		SendVertex(5, 4, 0, 0);
		SendVertex(3, 4, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

		SendVertex(2, 5, 0, 1);
		SendVertex(1, 5, 0, 2);
		SendVertex(6, 5, 0, 0);
		SendVertex(7, 5, 0, 3);

	GXEnd();  
}

/*---------------------------------------------------------------------------*
    Name:			DrawTriFanCube
    
    Description:	Draws the cube as Triangles  
    				
    Arguments:		none
    
    Returns:		none
 *---------------------------------------------------------------------------*/
void DrawTriFanCube( void )
{
	Mtx mv, r1, r2, t;

	MTXTrans(t, 250, 0, 0);
	MTXRotDeg(r1, 'X', (float)rot);
	MTXRotDeg(r2, 'y', (float)rot/2.0F);

	MTXConcat(r2, r1, mv);
	MTXConcat(t, mv, mv);
	MTXConcat(v, mv, mv);
	GXLoadPosMtxImm(mv, GX_PNMTX0);
	MTXInverse(mv, mv);
	MTXTranspose(mv, mv);
	GXLoadNrmMtxImm(mv, GX_PNMTX0);
	
	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

		SendVertex(0, 0, 1, 0);
		SendVertex(1, 0, 2, 1);
		SendVertex(2, 0, 3, 2);
		SendVertex(3, 0, 4, 3);

	GXEnd();

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

		SendVertex(4, 1, 0, 0);
		SendVertex(5, 1, 0, 1);
		SendVertex(6, 1, 0, 2);
		SendVertex(7, 1, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

		SendVertex(2, 2, 0, 0);
		SendVertex(6, 2, 0, 1);
		SendVertex(5, 2, 0, 2);
		SendVertex(3, 2, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

		SendVertex(1, 3, 0, 0);
		SendVertex(0, 3, 0, 1);
		SendVertex(4, 3, 0, 2);
		SendVertex(7, 3, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);   

		SendVertex(5, 4, 0, 0);
		SendVertex(4, 4, 0, 1);
		SendVertex(0, 4, 0, 2);
		SendVertex(3, 4, 0, 3);

	GXEnd();

	GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);

		SendVertex(6, 5, 0, 0);
		SendVertex(2, 5, 0, 1);
		SendVertex(1, 5, 0, 2);
		SendVertex(7, 5, 0, 3);

	GXEnd();  
}

/*---------------------------------------------------------------------------*/
static void SendVertex ( u16 posIndex, u16 normalIndex, u16 colorIndex, u16 texCoordIndex )
{
	//position section
	switch(PositionControl)
	{
		case 0:
		case 1:
		case 2:
			GXPosition1x16(posIndex);
			break;
		case 3:
		case 4:
		case 5:
			GXPosition1x8((u8)posIndex);
			break;
		case 6:
			GXPosition3f32(FloatVert[(posIndex * 3)], FloatVert[(posIndex * 3) + 1], FloatVert[(posIndex * 3) + 2]);
			break;
		case 7:
			GXPosition3s16(Vert16[(posIndex * 3)], Vert16[(posIndex * 3) + 1], Vert16[(posIndex * 3) + 2]);
			break;
		case 8:
			GXPosition3s8(Vert8[(posIndex * 3)], Vert8[(posIndex * 3) + 1], Vert8[(posIndex * 3) + 2]);
			break;
	}

	//normal section
	switch(NormalControl)
	{
		case 0:
			break;
		case 1:
		case 2:
		case 3:
			GXNormal1x16(normalIndex);
			break;
		case 4:
		case 5:
		case 6:
			GXNormal1x8((u8)normalIndex);
			break;
		case 7:
			GXNormal3f32(FloatNorm[(normalIndex * 3)], FloatNorm[(normalIndex * 3) + 1], FloatNorm[(normalIndex * 3) + 2]);
			break;
		case 8:
			GXNormal3s16(Norm16[(normalIndex * 3)], Norm16[(normalIndex * 3) + 1], Norm16[(normalIndex * 3) + 2]);
			break;
		case 9:
			GXNormal3s8(Norm8[(normalIndex * 3)], Norm8[(normalIndex * 3) + 1], Norm8[(normalIndex * 3) + 2]);
			break;
	}

	//color section
	switch(ColorControl)
	{
		case 0:
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			GXColor1x16(colorIndex);
			break;
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			GXColor1x8((u8)colorIndex);
			break;
		case 13:
			GXColor4u8(ColorRGBA8[(colorIndex * 4)], ColorRGBA8[(colorIndex * 4) + 1],
					   ColorRGBA8[(colorIndex * 4) + 2], ColorRGBA8[(colorIndex * 4) + 3]);
			break;
		case 14:
			GXColor1u32(((u32 *)ColorRGBA8)[colorIndex]);
			break;
		case 15:
			GXColor3u8(ColorRGBA6[(colorIndex * 3)], ColorRGBA6[(colorIndex * 3) + 1],
					   ColorRGBA6[(colorIndex * 3) + 2]);
			break;
		case 16:
			GXColor1u16(((u16 *)ColorRGBA4)[colorIndex]);
			break;
		case 17:
			GXColor4u8(ColorRGBX8[(colorIndex * 4)], ColorRGBX8[(colorIndex * 4) + 1],
					   ColorRGBX8[(colorIndex * 4) + 2], ColorRGBX8[(colorIndex * 4) + 3]);
			break;
		case 18:
			GXColor1u32(((u32 *)ColorRGBX8)[colorIndex]);
			break;
		case 19:
			GXColor3u8(ColorRGB8[(colorIndex * 3)], ColorRGB8[(colorIndex * 3) + 1],
					   ColorRGB8[(colorIndex * 3) + 2]);
			break;
		case 20:
			GXColor1u16(((u16 *)ColorRGB565)[colorIndex]);
			break;
	}

	//texCoord section
	switch(TexCoordControl)
	{
		case 0:
			break;
		case 1:
		case 2:
		case 3:
			GXTexCoord1x16(texCoordIndex);
			break;
		case 4:
		case 5:
		case 6:
			GXTexCoord1x8((u8)texCoordIndex);
			break;
		case 7:
			GXTexCoord2f32(FloatTex[(texCoordIndex * 2)], FloatTex[(texCoordIndex * 2) + 1]);
			break;
		case 8:
			GXTexCoord2u16(Tex16[(texCoordIndex * 2)], Tex16[(texCoordIndex * 2) + 1]);
			break;
		case 9:
			GXTexCoord2u8(Tex8[(texCoordIndex * 2)], Tex8[(texCoordIndex * 2) + 1]);
			break;
	}

}
