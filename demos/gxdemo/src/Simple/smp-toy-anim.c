/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     smp-toy-anim.c

  Copyright 1998, 1999 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/Simple/smp-toy-anim.c $    
    
    2     7/07/00 5:57p Dante
    PC Compatibility
    
    1     3/16/00 7:24p Alligator
    rename to match other demos conventions
    
    1     3/06/00 12:11p Alligator
    move from gxdemos to gxdemos/Simple
    
    4     9/15/99 3:00p Ryan
    update to fix compiler warnings
    
    3     7/23/99 2:51p Ryan
    changed dolphinDemo.h to demo.h
    
    2     7/23/99 12:15p Ryan
    included dolphinDemo.h
    
    1     6/17/99 1:25a Alligator
    simple interactive toy.  demonstrates texture api, multiple vertex
    formats and many primitive types.
    
  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#include <demo.h>
#include "toy-stick.h"
#include "toy-anim.h"

#define WALLX_BOUNCE            0.6F
#define WALLZ_BOUNCE            0.5F
#define OUT_CORNER_BOUNCE       1.2F
#define IN_CORNER_BOUNCE        0.4F
#define SLOPE_ACCEL             1.01F

#define BALL_CSIZEX             (BALL_SIZEX / 2.0F)
#define BALL_CSIZEZ             (BALL_SIZEZ / 2.0F)

#define AX_SCALE                0.01F
#define AY_SCALE                0.01F
#define VX_FRICTION             0.999F
#define VY_FRICTION             0.999F

/*---------------------------------------------------------------------------*
   Global variables
 *---------------------------------------------------------------------------*/
/*
 *  Flags for collision
 */
typedef enum 
{
    GO       = 0,
    NOGO     = 1,
    TELEPORT = 2,
    LANDED   = 4,
    ONSLOPE  = 8,
    FLIPA    = 16,
    FLIPB    = 32
} CollisionType;

/*
 *  Collision Symbol/Flag relationship
 */
static u32 CollSymTab[][2]=
{ 
    ' ',NOGO,
    '.',GO,
    'a',GO|FLIPA,
    'b',GO|FLIPB,
    '-',GO|LANDED,
    '|',GO|ONSLOPE,
    '<',GO|TELEPORT|ONSLOPE,
    '>',GO|TELEPORT|ONSLOPE,
    '\0',0
};

/*
 *  Collsion Grid
 *
 *  Two levels needed since bridges intersect.  In order to make
 *  collision logic easier, sloped bridge is straight in collision grid
 *  and the ball is translated to the proper slope when drawing.
 */
static unsigned char CollisionGrid[2][2*9][2*9] = 
{
  {
"                  ",
"                  ",
"  ...aaaa.bbbb..  ",
"  ...aaaa.bbbb..  ",
"  ..  ..      ..  ",
"  ..  ..      ..  ",
"  ..  ..      ..  ",
"  ..  ..      ..  ",
"  ..  ......  ..  ",
"  ..  ......  ..  ",
"  ..      ..  ..  ",
"  ..      ..  ..  ",
"  ..      ..  ..  ",
"  ..      ..  ..  ",
"  ..bbbb.aaaa...  ",
"  ..bbbb.aaaa...  ",
"                  ",
"                  "
  },

  {
"                  ",
"                  ",
"  ..aaaa.bbbb...  ",
"  ..aaaa.bbbb...  ",
"  ..      --  ..  ",
"  ..  >>  --  ..  ",
"  ..  ||      ..  ",
"  ..  ||      ..  ",
"  ..  ||      ..  ",
"  ..  ||      ..  ",
"  ..  ||      ..  ",
"  ..  ||      ..  ",
"  ..  ..      ..  ",
"  ..  ..      ..  ",
"  ...bbbb.aaaa..  ",
"  ...bbbb.aaaa..  ",
"                  ",
"                  "
  }
};


static Vec CurPos = {90.0F, 0.0F, 10.0F}; // current position
static u32 CurMask = 0;  // current collision mask at center of sphere
static u32 Teleport = 0; // teleporting this frame



/*---------------------------------------------------------------------------*
    Name:           GetCurPos
    
    Description:    Get current position
                    
    Arguments:      none
    
    Returns:        Vec, current position
 *---------------------------------------------------------------------------*/
Vec GetCurPos( void )
{
    return(CurPos);
}

/*---------------------------------------------------------------------------*
    Name:           OnSlope
    
    Description:    Returns 1 if center of ball is currently on the sloped
                    bridge.
                    
    Arguments:      none
    
    Returns:        u32, 1 = on slope, 0 = not on slope
 *---------------------------------------------------------------------------*/
u32 OnSlope( void )
{
    return(CurMask & ONSLOPE);
}

/*---------------------------------------------------------------------------*
    Name:           DoTeleport
    
    Description:    Returns 1 if ball will be telported this tick
                    
    Arguments:      none
    
    Returns:        u32, 1 = teleporting, 0 = not teleporting
 *---------------------------------------------------------------------------*/
u32 DoTeleport( void )
{
    return(Teleport);
}

/*---------------------------------------------------------------------------*
    Name:           TestPoint
    
    Description:    Test point for collision
                    
    Arguments:      curgrid   which collision grid to test
                    x         x coordinate to test
                    y         y coordinate to test
    
    Returns:        collision mask at point, u32
 *---------------------------------------------------------------------------*/
static u32 TestPoint( u32 curgrid, f32 x, f32 y )
{
    s32     xt;  // truncated x coordinate
    s32     yt;  // truncated y coordinate
    u32     i;
    u8      tilec;

    // stay inside NOGO border
    xt = (s32)((x + 20.0) / 10.0);
    yt = (s32)((y + 20.0) / 10.0);

    // clamp to table size
    if (xt < 0)  xt = 0;
    if (xt > 17) xt = 17;
    if (yt < 0)  yt = 0;
    if (yt > 17) yt = 17;

    tilec = CollisionGrid[curgrid][yt][xt];

    for (i = 0; CollSymTab[i][0]; i++) 
    {
        if (CollSymTab[i][0]==tilec) 
            return(CollSymTab[i][1]);
    }
    return(0);
}

/*---------------------------------------------------------------------------*
    Name:           AnimTick
    
    Description:    React to collision.  Computes next position and velocity.
                    
    Arguments:      none
 
    Returns:        none
 *---------------------------------------------------------------------------*/
void AnimTick( void )
{
    static u32 curGrid    = 0;  // current collision grid
    static Vec curVel     = { 0.0F, 0.0F, 0.0F };
    // four points to test collision against
    static Vec offsets[4] = {{-BALL_CSIZEX, 0.0F, -BALL_CSIZEZ},
                             { BALL_CSIZEX, 0.0F, -BALL_CSIZEZ},
                             { BALL_CSIZEX, 0.0F,  BALL_CSIZEZ},
                             {-BALL_CSIZEX, 0.0F,  BALL_CSIZEZ}};
    u32   i;             // loop variable
    u32   fullMask;      // all flags
    u32   wallMask = 0;  // only GO/NOGO bits
    Vec   tp;            // test point
    Vec   nextPos;
    Vec   nextVel;

    // switch collision grids? test center point
    CurMask = TestPoint(curGrid, CurPos.x, CurPos.z);
    if (CurMask & FLIPA)
        curGrid = 0;
    else if (CurMask & FLIPB)
        curGrid = 1;

    // all points must be in teleport area in order to teleport
    Teleport = TELEPORT;

    // build collision detection mask
    for (i = 0; i < 4; i++)
    {
        VECAdd(&CurPos, &curVel, &tp);
        VECAdd(&tp, &offsets[i], &tp);
        fullMask = TestPoint(curGrid, tp.x, tp.z);
        Teleport &= fullMask & TELEPORT; 
        wallMask |= ((fullMask & NOGO) ? 1 : 0) << i;
    }

    // teleport to the bottom of the ramp
    if (Teleport)
        CurPos.x -= (CurPos.z - SLOPE_ZSTART) * SLOPE_SLOPE;

    nextVel = curVel; // default

    switch(wallMask)
    {
        case 0x3: 
        case 0xc: // reflect velocity z
            nextVel.x = curVel.x;
            nextVel.z = -curVel.z * WALLZ_BOUNCE;
            break;

        case 0x9:
        case 0x6: // reflect velocity x
            nextVel.x = -curVel.x * WALLX_BOUNCE;
            nextVel.z = curVel.z;
            break;
        
        case 0x1:
            if (curVel.x < 0.0F) nextVel.x = -curVel.x * OUT_CORNER_BOUNCE;
            if (curVel.z < 0.0F) nextVel.z = -curVel.z * OUT_CORNER_BOUNCE;
            break;

        case 0x2:
            if (curVel.x > 0.0F) nextVel.x = -curVel.x * OUT_CORNER_BOUNCE;
            if (curVel.z < 0.0F) nextVel.z = -curVel.z * OUT_CORNER_BOUNCE;
            break;

        case 0x4:
            if (curVel.x > 0.0F) nextVel.x = -curVel.x * OUT_CORNER_BOUNCE;
            if (curVel.z > 0.0F) nextVel.z = -curVel.z * OUT_CORNER_BOUNCE;
            break;

        case 0x8:
            if (curVel.x < 0.0F) nextVel.x = -curVel.x * OUT_CORNER_BOUNCE;
            if (curVel.z > 0.0F) nextVel.z = -curVel.z * OUT_CORNER_BOUNCE;
            break;

        case 0x7:
        case 0xb:
        case 0xd:
        case 0xe: // invert velocity
            nextVel.x = -curVel.x * IN_CORNER_BOUNCE;
            nextVel.z = -curVel.z * IN_CORNER_BOUNCE;
            break;

        case 0x0: // no collision
            break;

        default:  // illegal case
            OSReport("Illegal case in React()\n");    
            break;
    }

    if ((CurMask & ONSLOPE) && (nextVel.z < 0.0F))
        nextVel.z *= SLOPE_ACCEL; // accelerate down slope

    VECAdd(&CurPos, &nextVel, &nextPos);

    CurPos = nextPos;
    curVel = nextVel;
    curVel.x += -GetAnalogX() * AX_SCALE;
    curVel.z += GetAnalogY() * AY_SCALE;
    curVel.x *= VX_FRICTION;
    curVel.z *= VY_FRICTION;
}
