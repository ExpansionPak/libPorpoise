/*---------------------------------------------------------------------------*
  Project:  libPorpoise
  File:     mtx.h
  
  Matrix and vector math functions.
  
  Based on Nintendo's Revolution SDK.
 *---------------------------------------------------------------------------*/

#ifndef DOLPHIN_MTX_H
#define DOLPHIN_MTX_H

#include <dolphin/os.h>
#include <dolphin/mtx/GeoTypes.h>
#include <dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Matrix macros */
#define MTX_PTR_OFFSET 3
#define MTX44_PTR_OFFSET 4
#define MTXDegToRad(a) ((a)*0.01745329252f)
#define MTXRadToDeg(a) ((a)*57.29577951f)
#define MTXRowCol(m, r, c) ((m)[(r)][(c)])

typedef struct {
    u32 numMtx;
    MtxPtr stackBase;
    MtxPtr stackPtr;
} MtxStack, *MtxStackPtr;

#define MTXAllocStack(sPtr, numMtx) \
    (((MtxStackPtr)(sPtr))->stackBase = (MtxPtr)OSAlloc(((numMtx) * sizeof(Mtx))))

#define MTXFreeStack(sPtr) \
    (OSFree((void*)(((MtxStackPtr)(sPtr))->stackBase)))

/* Matrix functions needed by onetri demo */
void MTXIdentity(Mtx m);
void MTXCopy(const Mtx src, Mtx dst);
void MTXConcat(const Mtx a, const Mtx b, Mtx ab);
void MTXTrans(Mtx m, f32 xT, f32 yT, f32 zT);
void MTXScale(Mtx m, f32 xS, f32 yS, f32 zS);
void MTXLookAt(Mtx m, const Point3d* camPos, const Vec* camUp, const Point3d* target);
void MTXFrustum(Mtx44 m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void MTXInitStack(MtxStackPtr sPtr, u32 numMtx);
MtxPtr MTXPush(MtxStackPtr sPtr, Mtx m);
MtxPtr MTXPushFwd(MtxStackPtr sPtr, Mtx m);
MtxPtr MTXPushInv(MtxStackPtr sPtr, Mtx m);
MtxPtr MTXPushInvXpose(MtxStackPtr sPtr, Mtx m);
MtxPtr MTXPop(MtxStackPtr sPtr);
MtxPtr MTXGetStackPtr(MtxStackPtr sPtr);
void MTXRotRad(Mtx m, char axis, f32 rad);
void MTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA);
void MTXRotDeg(Mtx m, char axis, f32 deg);
void MTXRotAxisRad(Mtx m, const Vec* axis, f32 rad);
void MTXRotAxisDeg(Mtx m, const Vec* axis, f32 deg);
void MTXQuat(Mtx m, const Quaternion* q);
void MTXPerspective(Mtx44 m, f32 fovY, f32 aspect, f32 n, f32 f);
void MTXOrtho(Mtx44 m, f32 top, f32 bottom, f32 left, f32 right, f32 nearZ, f32 farZ);
void MTXLightFrustum(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void MTXLightPerspective(Mtx m, f32 fovY, f32 aspect, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void MTXLightOrtho(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 scaleS, f32 scaleT, f32 transS, f32 transT);
void MTXReflect(Mtx m, const Point3d* p, const Vec* n);
void MTXMultVec(const Mtx m, const Vec* src, Vec* dst);
void MTXMultVecArray(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void MTXMultVecSR(const Mtx m, const Vec* src, Vec* dst);
void MTXMultVecArraySR(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void MTXTranspose(const Mtx src, Mtx dst);
u32  MTXInverse(const Mtx src, Mtx inv);
f32 VECDotProduct(const Vec* a, const Vec* b);
f32 VECMag(const Vec* v);
f32 VECSquareMag(const Vec* v);
f32 VECSquareDistance(const Vec* a, const Vec* b);
f32 VECDistance(const Vec* a, const Vec* b);
void VECCrossProduct(const Vec* a, const Vec* b, Vec* axb);
void VECNormalize(const Vec* src, Vec* dst);
void VECAdd(const Vec* a, const Vec* b, Vec* ab);
void VECSubtract(const Vec* a, const Vec* b, Vec* a_b);
void VECReflect(const Vec* src, const Vec* normal, Vec* dst);
void VECHalfAngle(const Vec* a, const Vec* b, Vec* half);
void VECScale(const Vec* src, Vec* dst, f32 scale);

/* Paired-single helpers */
void PSMTXIdentity(Mtx m);
void PSMTXCopy(const Mtx src, Mtx dst);
void PSMTXConcat(const Mtx a, const Mtx b, Mtx ab);
void PSMTXMultVecArray(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void PSMTXReorder(const Mtx src, ROMtx dest);
void PSMTXROMultVecArray(const ROMtx m, const Vec* srcBase, Vec* dstBase, u32 count);
void PSMTXTranspose(const Mtx src, Mtx dst);
u32  PSMTXInverse(const Mtx src, Mtx inv);
void PSMTXScale(Mtx m, f32 xS, f32 yS, f32 zS);

#ifdef __cplusplus
}
#endif

#endif /* DOLPHIN_MTX_H */
