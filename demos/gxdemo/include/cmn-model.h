/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     cmn-model.h

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/include/cmn-model.h $
    
    3     4/17/00 6:10p Hirose
    removed cmn-pad.h
    
    2     3/23/00 6:31p Hirose
    updated to use DEMOPad library instead of cmn-pad
    
    1     3/06/00 12:03p Alligator
    move from gx/tests to demos/gxdemos and rename
    
    4     11/17/99 1:24p Alligator
    removed instances of 'near' and 'far' for PC emulator port
    
    3     10/14/99 7:57p Yasu
    Change ViewPort structure to fit ViewPort/Scissor API
    
    2     9/10/99 6:48a Yasu
    Change variable types of member in ViewPort structure
    
    1     9/07/99 9:49p Yasu
    Initial version
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef	__CMD_MODEL_H__
#define	__CMD_MODEL_H__
#ifdef	__cplusplus
extern	"C" {
#endif

typedef struct {
    void	(*draw)();
    void	(*anim)();
    u32		flag;
}   Model;

typedef	struct	{
    Point3d	position;
    Point3d	target;
    Vec		up;
    float	fovy;
    float	znear;
    float	zfar;
    // Defined internally
    Mtx		viewMtx;
    Mtx44	projMtx;
}   Camera;

typedef	struct {
    s16		xorg;
    s16		yorg;
    s16		width;
    s16		height;
}   ViewPort;

// For cmPadStatus
//#include "cmn-pad.h" // Removed

extern	void	cmModelAnime( DEMOPadStatus*, Camera* );
extern	Model	cmModel[];

#define	MODEL_REFERRED	(1<<0)

#ifdef	__cplusplus
}
#endif
#endif	//__CMD_MODEL_H__

/*======== End of cmn-model.h ========*/
