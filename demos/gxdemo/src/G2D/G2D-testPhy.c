/*---------------------------------------------------------------------------*
  Project:  Dolphin
  File:     G2D-testPhy.c (Test of 2D API by Paul Donnelly, Nov. 1999)

  Copyright 1998, 1999, 2000 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: /Dolphin/build/demos/gxdemo/src/G2D/G2D-testPhy.c $
    
    3     3/24/00 5:41p Carl
    Used #defines for screen size.
    
    2     3/23/00 2:10a Hirose
    updated to use DEMOPad library / deleted all tab codes
    
    1     3/06/00 12:09p Alligator
    move from tests/gx and rename
    
    5     2/24/00 7:05p Yasu
    Rename gamepad key to match HW1
    
    4     99/12/15 2:14p Paul
    Added a switch to turn off map-editor mode
    
    3     99/12/14 8:34a Paul
    
    2     99/12/12 10:07p Paul
    
    1     99/12/09 12:28p Paul

 *---------------------------------------------------------------------------*/

#include <demo.h>
#include <math.h>

#include "G2D-test.h"

/*---------------------------------------------------------------------------*
   Defines
 *---------------------------------------------------------------------------*/
#define SIMULATION_FREQUENCY  75.0F
#define FRAME_RATE            75.0F

#define ANGULAR_DAMPING       0.94F
#define LINEAR_DAMPING        0.975F
#define ANGULAR_BRAKING       0.75F
#define LINEAR_BRAKING        0.85F

// Angular acceleration in radians per timestep-squared
//#define ANGULAR_ACCELERATION  (590.0F / (SIMULATION_FREQUENCY * SIMULATION_FREQUENCY))
#define ANGULAR_ACCELERATION  (10.0F / (SIMULATION_FREQUENCY * SIMULATION_FREQUENCY))

// Linear acceleration in world pixels per timestep-squared
#define LINEAR_ACCELERATION   (1500.0F /  (SIMULATION_FREQUENCY * SIMULATION_FREQUENCY))

#define ONE_OVER_127          0.007874F  // max/min controller (x,y) = +/- 127
#define PI_TIMES_2            6.283185307179586476925286766559F

#define MAX_ITNS1           4
#define COEF_RESTITUTION   -0.4F
#define VELOCITY_LOOKAHEAD  10.0F        // Ratio of velocity-per-timestep to camera lookahead point

/*---------------------------------------------------------------------------*
   Global Variables
 *---------------------------------------------------------------------------*/

G2DPosOri poCam;
G2DPosOri poShip;

s32 nStartIdx;
u32 nMode = 0;
u8 *map;
s32 nMapWidth, nMapHeight;
f32 rWorldWidth, rWorldHeight;

/*---------------------------------------------------------------------------*
   Static Global Variables
 *---------------------------------------------------------------------------*/

static f32 rVelX =  0.0F, rVelY = 0.0F;
static f32 rAngV = 0.0F;
f32 rAng = 0.0F;
static f32 rRelCamX, rRelCamY, rCamA;
static f32 rCVelX, rCVelY, rCVelA;
static f32 rCIntX, rCIntY, rCIntA;
static f32 rCamI;

u16 nButtons;
u16 nOldButtons = 0;
s8 stickX;
s8 stickY;

static s16 nViewportTlcX = 0;
static s16 nViewportTlcY = 0;
static s16 nViewportVelX = 1;
static s16 nViewportVelY = 1;

#if _OUTPUT
    #define MAX_RECORDS 8000

    typedef struct
    {
        f32 rPosX;
        f32 rPosY;
        f32 rVelX;
        f32 rVelY;
        f32 rAng;
        f32 rAngV;
    }
    Record;

    Record aRecord[MAX_RECORDS];

    FILE *fpOut;
#endif

static void Options ( u16 nButtons );
static void UpdatePosition( f32 rRadius );
static void HandleCollisions( f32 *pRemainingTime, f32 rRadius );
static __inline s32 Collide(s32 nMx, s32 nMy, f32 rRad, f32 *pTime, f32 *pNx, f32 *pNy );
static void MoveShip( void );


/*---------------------------------------------------------------------------*
    Name:           AnimTick

    Description:    Updates the objects in the world by one timestep

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void AnimateViewport()
{
    G2DSetViewport((u16)nViewportTlcX, (u16)nViewportTlcY, 
                   MY_SCREEN_WIDTH*3/4, MY_SCREEN_HEIGHT*3/4);

    nViewportTlcX += nViewportVelX;
    nViewportTlcY += nViewportVelY;

    if ((nViewportTlcX == 0) || (nViewportTlcX == MY_SCREEN_WIDTH*1/4))
    {
        nViewportVelX *= -1;
    }

    if ((nViewportTlcY == 0) || (nViewportTlcY == MY_SCREEN_HEIGHT*1/4))
    {
        nViewportVelY *= -1;
    }
}


static void PhysicsInit()
{
    /*  To handle different frame rates but achieve the same motion speeds
     *  we scale the acceleration and damping according to the following
     *  equations:
     *                                   FrameRatio
     *  NewAcc = Acc * Damping * (Damping           -  1)
     *           ----------------------------------------
     *                        (Damping - 1)
     *
     *
     *                      FrameRatio
     *  NewDamping = Damping
     *
     */

    f32 rRatio = SIMULATION_FREQUENCY * 0.01667F;

    //rLinearAcceleration = 
}


/*---------------------------------------------------------------------------*
    Name:           CameraInit

    Description:    Initialize the projection matrix and load into hardware.

    Arguments:      none
    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraInit      ( void )
{
    poCam.rPosX = poShip.rPosX + (VELOCITY_LOOKAHEAD * rVelX);
    poCam.rPosY = poShip.rPosY + (VELOCITY_LOOKAHEAD * rVelY);
    rRelCamX = 0.0F;
    rRelCamY = 0.0F;
    rCamA = rAng;
    rCVelX = 0.0F;
    rCVelY = 0.0F;
    rCVelA = 0.0F;
    rCIntX = 0.0F;
    rCIntY = 0.0F;
    rCIntA = 0.0F;
    rCamI = 0.9F;
}

/*---------------------------------------------------------------------------*
    Name:           CameraUpdate

    Description:    Updates the camera object based on the joystick's state.

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void CameraUpdate()
{
    f32 rCamDesiredX, rCamDesiredY, rCamDesiredA;

    // Camera motion implemented with a PID controller
#if _OUTPUT
    fprintf(fpOut, "Pos(%7.4f,%7.4f) Vel(%7.4f,%7.4f) Ang:%7.2f AngV:%7.3f Cam(%7.4f,%7.4f)\n",
        poShip.rPosX, poShip.rPosY, rVelX, rVelY, rAng, rAngV, poCam.rPosX, poCam.rPosY);
#endif

    rCamDesiredX = VELOCITY_LOOKAHEAD * rVelX;
    rCamDesiredY = VELOCITY_LOOKAHEAD * rVelY;
    rCamDesiredA = rAng;

    rCIntX = (rCamI * rCIntX) + ((1.0F - rCamI) * (rCamDesiredX - rRelCamX));
    rCIntY = (rCamI * rCIntY) + ((1.0F - rCamI) * (rCamDesiredY - rRelCamY));
    rCIntA = (rCamI * rCIntA) + ((1.0F - rCamI) * (rCamDesiredA - rCamA));

    rCVelX += ((rCamDesiredX - rRelCamX) * 0.04F) + (rCIntX * 0.05F);
    rCVelY += ((rCamDesiredY - rRelCamY) * 0.04F) + (rCIntY * 0.05F);
    rCVelA += ((rCamDesiredA - rCamA)    * 0.05F) + (rCIntA * 0.1F);

    rCVelX *= 0.4F;
    rCVelY *= 0.4F;
    rCVelA *= 0.4F;

    rRelCamX += rCVelX;
    rRelCamY += rCVelY;
    rCamA += rCVelA;

    poCam.rOriX = (f32)  sin(rCamA);
    poCam.rOriY = (f32) -cos(rCamA);

    poCam.rPosX = poShip.rPosX + rRelCamX;
    poCam.rPosY = poShip.rPosY + rRelCamY;

    if (poCam.rPosX >= rWorldWidth)  { poCam.rPosX -= rWorldWidth; }
    else if (poCam.rPosX < 0)        { poCam.rPosX += rWorldWidth; }
    if (poCam.rPosY >= rWorldHeight) { poCam.rPosY -= rWorldHeight; }
    else if (poCam.rPosY < 0)        { poCam.rPosY += rWorldHeight; }

    G2DSetCamera( &poCam );
}

static f32 rPx, rPy;
static f32 rVx, rVy;

static __inline s32 Collide(s32 nMx, s32 nMy, f32 rRad, f32 *pTime, f32 *pNx, f32 *pNy )
/*  Test for collision, and if collliding, do the collision response (bounce)
 *  return 1 for colliding, 0 for not colliding
 */
{
    s32 nTile;      // Index of tile (shifted left by 2 to index tileDesc array)
    s32 nCollType;  // Tile collision type
    s32 nRtn;       // Tile rotation (anticlockwise in 90deg steps)

    nTile = map[(nMx & (nMapWidth-1)) + ((nMy & (nMapHeight-1)) * nMapWidth)];
    nCollType = lyrBack.tileDesc[nTile].aUser[1];

    if (nCollType == 0)
    {
        // Non-collision tiles

        return 0;
    }

    if (nCollType == 3)
    {
        // Collision with a mid-section
        return 0;
    }

    // We will have acutal collisions to process from here
    nRtn = lyrBack.tileDesc[nTile].aUser[0];

    if (nCollType == 2)
    {
        // Collision with a border tile

        f32 rX, rY, rRatio;

        switch(nRtn)
        {
            case 0:
            {
                if (rVy <= 0.0F)
                {
                    return 0;
                }

                // line Y = nMy + 0.1
                rY = (f32)nMy + 0.1F - rRad;
                rRatio = (rY - rPy) / rVy;
                break;
            }
            case 1:
            {
                if (rVx <= 0.0F)
                {
                    return 0;
                }

                // line X = nMx + 0.1
                rX = (f32)nMx + 0.1F - rRad;
                rRatio = (rX - rPx) / rVx;
                break;
            }
            case 2:
            {
                if (rVy >= 0.0F)
                {
                    return 0;
                }

                //line Y = nMy + 0.9
                rY = (f32)nMy + 0.9F + rRad;
                rRatio = (rY - rPy) / rVy;
                break;
            }
            case 3:
            {
                if (rVx >= 0.0F)
                {
                    return 0;
                }

                // line X = nMx + 0.9
                rX = (f32)nMx + 0.9F + rRad;
                rRatio = (rX - rPx) / rVx;
                break;
            }
        }

        if ((rRatio < *pTime) && (rRatio > 0.0F))
        {
            *pTime = rRatio;

            if (nRtn & 1)
            {
                *pNx = 1.0F;
                *pNy = 0.0F;
            }
            else
            {
                *pNx = 0.0F;
                *pNy = 1.0F;
            }

            return 1;
        }

        return 0;
    }

    //  The remaining tiles are conic sections
    {
        f32 rCx, rCy;
        f32 rDx, rDy;
        f32 rA, rB, rC, rD;
        f32 rT1;

        // Find centre of conic section (rCx, rCy)

        rCx = (f32)nMx;
        rCy = (f32)nMy;

        if (nRtn < 2)
        {
            rCx += 1.0F;
        }

        if (((nRtn+1)&3) < 2)
        {
            rCy += 1.0F;
        }

        // Calculate roots of quadratic

        rDx = rCx - rPx;
        rDy = rCy - rPy;

        rA = (rVx * rVx) + (rVy * rVy);
        rB = (rRad + 0.9F) * (rRad + 0.9F) * rA;
        rC = (rDy * rVx) - (rDx * rVy);
        rD = rB - (rC * rC);
        if (rD < 0.0F)
        {
            return 0;
        }
        rD = (f32)sqrt(rD);

        rA = 1.0F / rA;
        rB = (rDx * rVx) + (rDy * rVy);
        rT1 = rA * (rB - rD);
        //rT2 = rA * (rB + rD);

        if (rT1 < 0.0F)
        {
            // We must be starting from inside the circle
            return 0;
        }

        if (rT1 < *pTime)
        {
            *pTime = rT1;

            // Now to calculate the normal at the point of collision
            *pNx = rPx + (rT1 * rVx) - rCx;
            *pNy = rPy + (rT1 * rVy) - rCy;

            // The normal must be normalized
            rA = 1.0F / (f32)sqrt((*pNx * *pNx) + (*pNy * *pNy));
            *pNx *= rA;
            *pNy *= rA;

            return 1;
        }

        return 0;
    }
}


static void HandleCollisions( f32 *pRemainingTime, f32 rRadius )
{
    s32 nIncX, nIncY;   // Offsets to possibly overlapping map coordinates
    f32 rNewPosX, rNewPosY;
    s32 nMx, nMy;
    f32 rFx, rFy;
    f32 rTime = 1.0F;
    f32 rNx, rNy;   // Collision Normal

    rNewPosX = rPx + (*pRemainingTime * rVx);
    rNewPosY = rPy + (*pRemainingTime * rVy);

    nMx = (s32)rNewPosX;
    nMy = (s32)rNewPosY;

    rFx = rNewPosX - nMx;
    rFy = rNewPosY - nMy;

    nIncX = -1;
    nIncY = -1;

    if (rFx > 0.5F)
    {
        rFx = 1.0F - rFx;
        nIncX = 1;
    }

    if (rFy > 0.5F)
    {
        rFy = 1.0F - rFy;
        nIncY = 1;
    }

    /*  Potential collision with centre tile (after step)
     */
    Collide( nMx, nMy, rRadius, &rTime, &rNx, &rNy );

    if (rFx < rRadius)
    {
        // Potential collision with horizontally adjacent tile
        Collide( nMx + nIncX, nMy, rRadius, &rTime, &rNx, &rNy );

        if (rFy < rRadius)
        {
            /*  Potential collisions with vertically adjacent and
             *  diagonally adjacent tiles
             */

            // Potential collision with vertically adjacent tile
            Collide( nMx, nMy + nIncY, rRadius, &rTime, &rNx, &rNy );

            // Potential collision with diagonally adjacent tile
            Collide( nMx + nIncX, nMy + nIncY, rRadius, &rTime, &rNx, &rNy );
        }
    }
    else if (rFy < rRadius)
    {
        // Potential collision with vertically adjacent tile
        Collide( nMx, nMy + nIncY, rRadius, &rTime, &rNx, &rNy );
    }

    if (rTime < *pRemainingTime)
    {
        f32 rN, rT;

        // We bumped into something within the timestep
        rTime -= 0.01F; // avoids numerical accuracy problems
        rPx += (rTime * rVx);
        rPy += (rTime * rVy);
        *pRemainingTime -= rTime;

        // Reflect normal component of velocity
        // Now resolve velocity in normal and tangential directions
        rN = (rVx * rNx) + (rVy * rNy); // Normal
        rT = (rVx * rNy) - (rVy * rNx); // Tangent

        // Scale normal velocity by coefficient of restitution
        rN *= COEF_RESTITUTION;

        // Reconsitute new velocity
        rVx = (rN * rNx) + (rT * rNy);
        rVy = (rN * rNy) - (rT * rNx);

        return;
    }

    // We didn't bump into anything
    rPx += (*pRemainingTime * rVx);
    rPy += (*pRemainingTime * rVy);
    *pRemainingTime = 0.0F;
}


/*---------------------------------------------------------------------------*
    Name:           UpdatePosition

    Description:    Update position of ship taking collisions into consideration

    Arguments:      rRadius    Radius of collision circle

    Returns:        none
 *---------------------------------------------------------------------------*/
static void UpdatePosition( f32 rRadius )
{
    f32 rStepRemaining = 1.0F;
    s32 nItns1;

    rPx = poShip.rPosX / lyrBack.nTileWidth;    // The collision routines won't really look
    rPy = poShip.rPosY / lyrBack.nTileHeight;   // correct unless TILE_WIDTH_LYR1 == TILE_HEIGHT_LYR1
    rVx = rVelX / lyrBack.nTileWidth;
    rVy = rVelY / lyrBack.nTileHeight;

    for(nItns1=0; nItns1 < MAX_ITNS1; nItns1++)
    {
        // Count collisions at end of step
        HandleCollisions( &rStepRemaining, rRadius );

        if (rStepRemaining < 0.01)
        {
            break;
        }
    }

    poShip.rPosX = rPx * lyrBack.nTileWidth;    // The collision routines won't really look
    poShip.rPosY = rPy * lyrBack.nTileHeight;   // correct unless TILE_WIDTH_LYR1 == TILE_HEIGHT_LYR1
    rVelX = rVx * lyrBack.nTileWidth;
    rVelY = rVy * lyrBack.nTileHeight;
}


void AnimInit( void )
{
    CameraInit();

    poShip.rOriX = (f32) sin(rAng);
    poShip.rOriY = (f32) -cos(rAng);

    map = lyrBack.map;
    rWorldWidth = (f32)(lyrBack.nTileWidth << lyrBack.nHS);
    rWorldHeight = (f32)(lyrBack.nTileHeight << lyrBack.nVS);
    nMapWidth = 1<<lyrBack.nHS;
    nMapHeight = 1<<lyrBack.nVS;

#if _OUTPUT
    fpOut = fopen("out.txt", "wb");
#endif
}


/*---------------------------------------------------------------------------*
    Name:           MoveShip

    Description:    Updates the ship by one timestep

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
static void MoveShip( void )
{
    static u32 nGameTime = 0;

    if (nButtons & PAD_TRIGGER_L)
    {
        rAngV = rAngV - ANGULAR_ACCELERATION;
    }
    if (nButtons & PAD_TRIGGER_R)
    {
        rAngV = rAngV + ANGULAR_ACCELERATION;
    }

    //rAngV = rAngV + stickX * 0.01575F;

/*  if (nButtons & PAD_BUTTON_A)
    {
        rVelX -= (poShip.rOriX * 0.04F);
        rVelY -= (poShip.rOriY * 0.04F);
    }
*/
    rVelX += (poShip.rOriX * LINEAR_ACCELERATION * ONE_OVER_127 * stickY);
    rVelY += (poShip.rOriY * LINEAR_ACCELERATION * ONE_OVER_127 * stickY);

    if (nButtons & PAD_BUTTON_B)
    {
        rVelX *= LINEAR_BRAKING;
        rVelY *= LINEAR_BRAKING;
        rAngV *= ANGULAR_BRAKING;
    }
    else
    {
        rVelX *= LINEAR_DAMPING;
        rVelY *= LINEAR_DAMPING;
        rAngV *= ANGULAR_DAMPING;
    }

    // Orientation can be updated trivially
    rAng += rAngV;
    if (rAng > PI_TIMES_2) { rAng -= PI_TIMES_2; rCamA -= PI_TIMES_2; }
    if (rAng < 0) { rAng += PI_TIMES_2; rCamA += PI_TIMES_2; }

    poShip.rOriX = (f32)  sin(rAng);
    poShip.rOriY = (f32) -cos(rAng);

    // Position must be updated carefully checking for collisions
    UpdatePosition( 0.4F );

    if (poShip.rPosX >= rWorldWidth)  { poShip.rPosX -= rWorldWidth; }
    else if (poShip.rPosX < 0)        { poShip.rPosX += rWorldWidth; }
    if (poShip.rPosY >= rWorldHeight) { poShip.rPosY -= rWorldHeight; }
    else if (poShip.rPosY < 0)        { poShip.rPosY += rWorldHeight; }
#if _OUTPUT
    aRecord[nGameTime].rPosX = poShip.rPosX;
    aRecord[nGameTime].rPosY = poShip.rPosY;
    aRecord[nGameTime].rVelX = rVelX;
    aRecord[nGameTime].rVelY = rVelY;
    aRecord[nGameTime].rAng  = rAng;
    aRecord[nGameTime].rAngV = rAngV;
    if (nGameTime >= MAX_RECORDS)
    {
        fclose(fpOut);
        fpOut = fopen("out.txt", "wb");
        nGameTime=0;
    }
#endif
    nGameTime++;
}


/*---------------------------------------------------------------------------*
    Name:           AnimTick

    Description:    Updates the objects in the world by one timestep

    Arguments:      none

    Returns:        none
 *---------------------------------------------------------------------------*/
void AnimTick( void )
{
    /*  Read controller
     */
    nButtons = DEMOPadGetButton(0);
    stickX = DEMOPadGetStickX(0);
    stickY = DEMOPadGetStickY(0);

    if (nMode == 2)
    {
        MapEditor( &lyrBack );
    }
    else
    {
        MoveShip();
    }

    if ((nButtons & ~nOldButtons) & PAD_BUTTON_Y)
    {
        if (nMode == 2)
        {
            /* Save edits to map */
            SaveMap( &lyrBack );
        }

#ifdef _EDITOR
        nMode = (nMode+1)%3;

        if (nMode == 2)
        {
            rAng = 0.0F;
            poShip.rOriX = (f32)  sin(rAng);
            poShip.rOriY = (f32) -cos(rAng);
            aEditStamp[0] = '.'-' ';
        }
#else
        nMode = (nMode+1)%2;
#endif // ifdef _EDITOR

        if (nMode != 1)
        {
            G2DSetViewport( 0, 0, MY_SCREEN_WIDTH, MY_SCREEN_HEIGHT );
        }
    }

    CameraUpdate();
    if (nMode == 1)
    {
        AnimateViewport();
    }

    nOldButtons = nButtons;
}
