/*---------------------------------------------------------------------------*
  Project:  libPorpoise
  File:     mtx.h
  
  Matrix and vector math functions.
  
  Based on Nintendo's Revolution SDK.
 *---------------------------------------------------------------------------*/

#ifndef DOLPHIN_MTX_H
#define DOLPHIN_MTX_H

#include <dolphin/mtx/GeoTypes.h>
#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Matrix macros */
#define MTXDegToRad(a) ((a)*0.01745329252f)
#define MTXRadToDeg(a) ((a)*57.29577951f)
#define MTXRowCol(m, r, c) ((m)[(r)][(c)])

/* Matrix functions needed by onetri demo */
void MTXIdentity(Mtx m);
void MTXConcat(const Mtx a, const Mtx b, Mtx ab);
void MTXTrans(Mtx m, f32 xT, f32 yT, f32 zT);
void MTXLookAt(Mtx m, const Point3d* camPos, const Vec* camUp, const Point3d* target);
void MTXFrustum(Mtx44 m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);

#ifdef __cplusplus
}
#endif

#endif /* DOLPHIN_MTX_H */

