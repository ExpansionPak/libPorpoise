/*---------------------------------------------------------------------------*
  Project:  libPorpoise
  File:     mtx.c
  
  Matrix and vector math functions.
  
  Based on Nintendo's Revolution SDK and Aurora implementation.
 *---------------------------------------------------------------------------*/

#include <dolphin/mtx.h>
#include <string.h>
#include <math.h>

/*---------------------------------------------------------------------------*
    Name:           MTXIdentity
    
    Description:    Creates an identity matrix (3x4).
    
    Arguments:      m      output matrix
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXIdentity(Mtx m) {
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = 0.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXCopy

    Description:    Copies a 3x4 matrix.

    Arguments:      src    source matrix
                    dst    destination matrix

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXCopy(const Mtx src, Mtx dst) {
    memcpy(dst, src, sizeof(Mtx));
}

/*---------------------------------------------------------------------------*
    Name:           MTXConcat
    
    Description:    Concatenates two matrices: ab = a * b
    
    Arguments:      a      first matrix
                    b      second matrix
                    ab     output matrix (can be same as a or b)
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXConcat(const Mtx a, const Mtx b, Mtx ab) {
    Mtx mTmp;
    MtxPtr m;
    
    if (ab == a || ab == b) {
        m = mTmp;
    } else {
        m = ab;
    }
    
    m[0][0] = a[0][2] * b[2][0] + ((a[0][0] * b[0][0]) + (a[0][1] * b[1][0]));
    m[0][1] = a[0][2] * b[2][1] + ((a[0][0] * b[0][1]) + (a[0][1] * b[1][1]));
    m[0][2] = a[0][2] * b[2][2] + ((a[0][0] * b[0][2]) + (a[0][1] * b[1][2]));
    m[0][3] = a[0][3] + (a[0][2] * b[2][3] + (a[0][0] * b[0][3] + (a[0][1] * b[1][3])));
    
    m[1][0] = a[1][2] * b[2][0] + ((a[1][0] * b[0][0]) + (a[1][1] * b[1][0]));
    m[1][1] = a[1][2] * b[2][1] + ((a[1][0] * b[0][1]) + (a[1][1] * b[1][1]));
    m[1][2] = a[1][2] * b[2][2] + ((a[1][0] * b[0][2]) + (a[1][1] * b[1][2]));
    m[1][3] = a[1][3] + (a[1][2] * b[2][3] + (a[1][0] * b[0][3] + (a[1][1] * b[1][3])));
    
    m[2][0] = a[2][2] * b[2][0] + ((a[2][0] * b[0][0]) + (a[2][1] * b[1][0]));
    m[2][1] = a[2][2] * b[2][1] + ((a[2][0] * b[0][1]) + (a[2][1] * b[1][1]));
    m[2][2] = a[2][2] * b[2][2] + ((a[2][0] * b[0][2]) + (a[2][1] * b[1][2]));
    m[2][3] = a[2][3] + (a[2][2] * b[2][3] + (a[2][0] * b[0][3] + (a[2][1] * b[1][3])));
    
    if (m == mTmp) {
        memcpy(ab, mTmp, sizeof(Mtx));
    }
}

/*---------------------------------------------------------------------------*
    Name:           MTXTrans
    
    Description:    Creates a translation matrix.
    
    Arguments:      m      output matrix
                    xT     translation in X
                    yT     translation in Y
                    zT     translation in Z
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXTrans(Mtx m, f32 xT, f32 yT, f32 zT) {
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = xT;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = yT;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = zT;
}

/*---------------------------------------------------------------------------*
    Name:           MTXScale

    Description:    Creates a scale matrix.

    Arguments:      m      output matrix
                    xS     scale in X
                    yS     scale in Y
                    zS     scale in Z

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXScale(Mtx m, f32 xS, f32 yS, f32 zS) {
    m[0][0] = xS;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = yS;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = zS;
    m[2][3] = 0.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXRotTrig

    Description:    Creates a rotation matrix around X/Y/Z using precomputed
                    sine/cosine values.

    Arguments:      m      output matrix
                    axis   'x','y','z' (case-insensitive)
                    sinA   sine of angle
                    cosA   cosine of angle

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXRotTrig(Mtx m, char axis, f32 sinA, f32 cosA) {
    MTXIdentity(m);
    switch (axis) {
        case 'x':
        case 'X':
            m[1][1] = cosA;
            m[1][2] = -sinA;
            m[2][1] = sinA;
            m[2][2] = cosA;
            break;
        case 'y':
        case 'Y':
            m[0][0] = cosA;
            m[0][2] = sinA;
            m[2][0] = -sinA;
            m[2][2] = cosA;
            break;
        case 'z':
        case 'Z':
            m[0][0] = cosA;
            m[0][1] = -sinA;
            m[1][0] = sinA;
            m[1][1] = cosA;
            break;
        default:
            break;
    }
}

/*---------------------------------------------------------------------------*
    Name:           MTXRotRad

    Description:    Creates a rotation matrix around X/Y/Z from radians.

    Arguments:      m      output matrix
                    axis   'x','y','z' (case-insensitive)
                    rad    angle in radians

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXRotRad(Mtx m, char axis, f32 rad) {
    MTXRotTrig(m, axis, sinf(rad), cosf(rad));
}

/*---------------------------------------------------------------------------*
    Name:           MTXRotDeg

    Description:    Creates a rotation matrix around X/Y/Z from degrees.

    Arguments:      m      output matrix
                    axis   'x','y','z' (case-insensitive)
                    deg    angle in degrees

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXRotDeg(Mtx m, char axis, f32 deg) {
    MTXRotRad(m, axis, MTXDegToRad(deg));
}

/*---------------------------------------------------------------------------*
    Name:           MTXRotAxisRad

    Description:    Creates a rotation matrix around an arbitrary axis.

    Arguments:      m      output matrix
                    axis   rotation axis (does not need to be unit length)
                    rad    angle in radians

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXRotAxisRad(Mtx m, const Vec* axis, f32 rad) {
    f32 x = axis->x;
    f32 y = axis->y;
    f32 z = axis->z;
    f32 len = sqrtf(x * x + y * y + z * z);
    f32 s;
    f32 c;
    f32 t;

    if (len <= 0.0f) {
        MTXIdentity(m);
        return;
    }

    x /= len;
    y /= len;
    z /= len;

    s = sinf(rad);
    c = cosf(rad);
    t = 1.0f - c;

    m[0][0] = t * x * x + c;
    m[0][1] = t * x * y - s * z;
    m[0][2] = t * x * z + s * y;
    m[0][3] = 0.0f;

    m[1][0] = t * x * y + s * z;
    m[1][1] = t * y * y + c;
    m[1][2] = t * y * z - s * x;
    m[1][3] = 0.0f;

    m[2][0] = t * x * z - s * y;
    m[2][1] = t * y * z + s * x;
    m[2][2] = t * z * z + c;
    m[2][3] = 0.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXRotAxisDeg

    Description:    Creates a rotation matrix around an arbitrary axis.

    Arguments:      m      output matrix
                    axis   rotation axis
                    deg    angle in degrees

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXRotAxisDeg(Mtx m, const Vec* axis, f32 deg) {
    MTXRotAxisRad(m, axis, MTXDegToRad(deg));
}

/*---------------------------------------------------------------------------*
    Name:           MTXQuat

    Description:    Creates a rotation matrix from quaternion (x,y,z,w).

    Arguments:      m      output matrix
                    q      quaternion

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXQuat(Mtx m, const Quaternion* q) {
    f32 x = q->x;
    f32 y = q->y;
    f32 z = q->z;
    f32 w = q->w;
    f32 n = x * x + y * y + z * z + w * w;

    if (n <= 0.0f) {
        MTXIdentity(m);
        return;
    }

    {
        f32 s = 2.0f / n;
        f32 xx = x * x * s;
        f32 yy = y * y * s;
        f32 zz = z * z * s;
        f32 xy = x * y * s;
        f32 xz = x * z * s;
        f32 yz = y * z * s;
        f32 wx = w * x * s;
        f32 wy = w * y * s;
        f32 wz = w * z * s;

        m[0][0] = 1.0f - (yy + zz);
        m[0][1] = xy - wz;
        m[0][2] = xz + wy;
        m[0][3] = 0.0f;

        m[1][0] = xy + wz;
        m[1][1] = 1.0f - (xx + zz);
        m[1][2] = yz - wx;
        m[1][3] = 0.0f;

        m[2][0] = xz - wy;
        m[2][1] = yz + wx;
        m[2][2] = 1.0f - (xx + yy);
        m[2][3] = 0.0f;
    }
}

static BOOL MtxStackCanPush(const MtxStackPtr sPtr) {
    if (sPtr == NULL || sPtr->stackBase == NULL || sPtr->numMtx == 0) {
        return FALSE;
    }

    if (sPtr->stackPtr == NULL) {
        return TRUE;
    }

    return ((u32)((sPtr->stackPtr - sPtr->stackBase) / MTX_PTR_OFFSET) < (sPtr->numMtx - 1));
}

/*---------------------------------------------------------------------------*
    Name:           MTXInitStack

    Description:    Initializes matrix stack metadata after MTXAllocStack.

    Arguments:      sPtr    target stack
                    numMtx  number of matrices allocated to stackBase

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXInitStack(MtxStackPtr sPtr, u32 numMtx) {
    if (sPtr == NULL) {
        return;
    }

    sPtr->numMtx = numMtx;
    sPtr->stackPtr = NULL;
}

/*---------------------------------------------------------------------------*
    Name:           MTXGetStackPtr

    Description:    Returns current top-of-stack matrix pointer.

    Arguments:      sPtr    matrix stack

    Returns:        top-of-stack pointer (or NULL if empty)
 *---------------------------------------------------------------------------*/
MtxPtr MTXGetStackPtr(MtxStackPtr sPtr) {
    return sPtr ? sPtr->stackPtr : NULL;
}

/*---------------------------------------------------------------------------*
    Name:           MTXPush

    Description:    Pushes a direct copy of m onto the matrix stack.

    Arguments:      sPtr    matrix stack
                    m       matrix to copy

    Returns:        pointer to pushed matrix (or current pointer on overflow)
 *---------------------------------------------------------------------------*/
MtxPtr MTXPush(MtxStackPtr sPtr, Mtx m) {
    MtxPtr dst;

    if (!MtxStackCanPush(sPtr)) {
        ASSERTMSG(FALSE, "MTXPush: stack overflow or uninitialized stack");
        return sPtr ? sPtr->stackPtr : NULL;
    }

    dst = (sPtr->stackPtr == NULL) ? sPtr->stackBase : (sPtr->stackPtr + MTX_PTR_OFFSET);
    MTXCopy(m, dst);
    sPtr->stackPtr = dst;
    return sPtr->stackPtr;
}

/*---------------------------------------------------------------------------*
    Name:           MTXPushFwd

    Description:    Pushes forward composite matrix (top * m).

    Arguments:      sPtr    matrix stack
                    m       matrix to post-concatenate

    Returns:        pointer to pushed matrix (or current pointer on overflow)
 *---------------------------------------------------------------------------*/
MtxPtr MTXPushFwd(MtxStackPtr sPtr, Mtx m) {
    MtxPtr dst;

    if (!MtxStackCanPush(sPtr)) {
        ASSERTMSG(FALSE, "MTXPushFwd: stack overflow or uninitialized stack");
        return sPtr ? sPtr->stackPtr : NULL;
    }

    if (sPtr->stackPtr == NULL) {
        MTXCopy(m, sPtr->stackBase);
        sPtr->stackPtr = sPtr->stackBase;
        return sPtr->stackPtr;
    }

    dst = sPtr->stackPtr + MTX_PTR_OFFSET;
    MTXConcat(sPtr->stackPtr, m, dst);
    sPtr->stackPtr = dst;
    return sPtr->stackPtr;
}

/*---------------------------------------------------------------------------*
    Name:           MTXPushInv

    Description:    Pushes inverse composite matrix (inverse(m) * top).

    Arguments:      sPtr    matrix stack
                    m       matrix whose inverse is pre-concatenated

    Returns:        pointer to pushed matrix (or current pointer on failure)
 *---------------------------------------------------------------------------*/
MtxPtr MTXPushInv(MtxStackPtr sPtr, Mtx m) {
    MtxPtr dst;
    Mtx mTmp;
    Mtx invTmp;

    if (!MtxStackCanPush(sPtr)) {
        ASSERTMSG(FALSE, "MTXPushInv: stack overflow or uninitialized stack");
        return sPtr ? sPtr->stackPtr : NULL;
    }

    if (sPtr->stackPtr == NULL) {
        MTXCopy(m, sPtr->stackBase);
        sPtr->stackPtr = sPtr->stackBase;
        return sPtr->stackPtr;
    }

    MTXCopy(m, mTmp);
    if (!MTXInverse(mTmp, invTmp)) {
        ASSERTMSG(FALSE, "MTXPushInv: MTXInverse failed (singular matrix)");
        return sPtr->stackPtr;
    }

    dst = sPtr->stackPtr + MTX_PTR_OFFSET;
    MTXConcat(invTmp, sPtr->stackPtr, dst);
    sPtr->stackPtr = dst;
    return sPtr->stackPtr;
}

/*---------------------------------------------------------------------------*
    Name:           MTXPushInvXpose

    Description:    Pushes inverse-transpose forward composite matrix
                    (top * transpose(inverse(m))).

    Arguments:      sPtr    matrix stack
                    m       matrix to inverse-transpose and concatenate

    Returns:        pointer to pushed matrix (or current pointer on failure)
 *---------------------------------------------------------------------------*/
MtxPtr MTXPushInvXpose(MtxStackPtr sPtr, Mtx m) {
    MtxPtr dst;
    Mtx mTmp;
    Mtx invTmp;
    Mtx invXposeTmp;

    if (!MtxStackCanPush(sPtr)) {
        ASSERTMSG(FALSE, "MTXPushInvXpose: stack overflow or uninitialized stack");
        return sPtr ? sPtr->stackPtr : NULL;
    }

    if (sPtr->stackPtr == NULL) {
        MTXCopy(m, sPtr->stackBase);
        sPtr->stackPtr = sPtr->stackBase;
        return sPtr->stackPtr;
    }

    MTXCopy(m, mTmp);
    if (!MTXInverse(mTmp, invTmp)) {
        ASSERTMSG(FALSE, "MTXPushInvXpose: MTXInverse failed (singular matrix)");
        return sPtr->stackPtr;
    }
    MTXTranspose(invTmp, invXposeTmp);

    dst = sPtr->stackPtr + MTX_PTR_OFFSET;
    MTXConcat(sPtr->stackPtr, invXposeTmp, dst);
    sPtr->stackPtr = dst;
    return sPtr->stackPtr;
}

/*---------------------------------------------------------------------------*
    Name:           MTXPop

    Description:    Pops one matrix from the stack.

    Arguments:      sPtr    matrix stack

    Returns:        new top-of-stack pointer, or NULL if stack becomes empty
 *---------------------------------------------------------------------------*/
MtxPtr MTXPop(MtxStackPtr sPtr) {
    if (sPtr == NULL || sPtr->stackPtr == NULL) {
        return NULL;
    }

    if (sPtr->stackPtr == sPtr->stackBase) {
        sPtr->stackPtr = NULL;
    } else {
        sPtr->stackPtr -= MTX_PTR_OFFSET;
    }

    return sPtr->stackPtr;
}

/*---------------------------------------------------------------------------*
    Name:           MTXMultVec

    Description:    Multiplies a vector by a 3x4 matrix (includes translation).

    Arguments:      m      matrix
                    src    source vector
                    dst    destination vector

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXMultVec(const Mtx m, const Vec* src, Vec* dst) {
    f32 x = src->x;
    f32 y = src->y;
    f32 z = src->z;

    dst->x = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
    dst->y = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
    dst->z = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
}

/*---------------------------------------------------------------------------*
    Name:           MTXMultVecArray

    Description:    Multiplies an array of vectors by a 3x4 matrix
                    (includes translation).

    Arguments:      m        matrix
                    srcBase  source vector array
                    dstBase  destination vector array
                    count    number of vectors

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXMultVecArray(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count) {
    while (count--) {
        MTXMultVec(m, srcBase, dstBase);
        ++srcBase;
        ++dstBase;
    }
}

/*---------------------------------------------------------------------------*
    Name:           MTXMultVecSR

    Description:    Multiplies a vector by the 3x3 (scale-rotate) part only.

    Arguments:      m      matrix
                    src    source vector
                    dst    destination vector

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXMultVecSR(const Mtx m, const Vec* src, Vec* dst) {
    f32 x = src->x;
    f32 y = src->y;
    f32 z = src->z;

    dst->x = m[0][0] * x + m[0][1] * y + m[0][2] * z;
    dst->y = m[1][0] * x + m[1][1] * y + m[1][2] * z;
    dst->z = m[2][0] * x + m[2][1] * y + m[2][2] * z;
}

/*---------------------------------------------------------------------------*
    Name:           MTXMultVecArraySR

    Description:    Multiplies an array of vectors by the 3x3 (scale-rotate)
                    part of a matrix only (no translation).

    Arguments:      m        matrix
                    srcBase  source vector array
                    dstBase  destination vector array
                    count    number of vectors

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXMultVecArraySR(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count) {
    while (count--) {
        MTXMultVecSR(m, srcBase, dstBase);
        ++srcBase;
        ++dstBase;
    }
}

/*---------------------------------------------------------------------------*
    Name:           VECDotProduct

    Description:    Returns the dot product of vectors a and b.

    Arguments:      a, b   input vectors

    Returns:        dot product
 *---------------------------------------------------------------------------*/
f32 VECDotProduct(const Vec* a, const Vec* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

/*---------------------------------------------------------------------------*
    Name:           VECMag

    Description:    Returns vector magnitude.

    Arguments:      v      input vector

    Returns:        magnitude
 *---------------------------------------------------------------------------*/
f32 VECMag(const Vec* v) {
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

/*---------------------------------------------------------------------------*
    Name:           VECSquareMag

    Description:    Returns square magnitude of a vector.

    Arguments:      v      input vector

    Returns:        square magnitude
 *---------------------------------------------------------------------------*/
f32 VECSquareMag(const Vec* v) {
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

/*---------------------------------------------------------------------------*
    Name:           VECCrossProduct

    Description:    Cross product: axb = a x b

    Arguments:      a,b    input vectors
                    axb    output vector

    Returns:        none
 *---------------------------------------------------------------------------*/
void VECCrossProduct(const Vec* a, const Vec* b, Vec* axb) {
    axb->x = a->y * b->z - a->z * b->y;
    axb->y = a->z * b->x - a->x * b->z;
    axb->z = a->x * b->y - a->y * b->x;
}

/*---------------------------------------------------------------------------*
    Name:           VECSquareDistance

    Description:    Returns square distance between vectors.

    Arguments:      a, b   input vectors

    Returns:        square distance
 *---------------------------------------------------------------------------*/
f32 VECSquareDistance(const Vec* a, const Vec* b) {
    f32 dx = a->x - b->x;
    f32 dy = a->y - b->y;
    f32 dz = a->z - b->z;
    return dx * dx + dy * dy + dz * dz;
}

/*---------------------------------------------------------------------------*
    Name:           VECDistance

    Description:    Returns distance between vectors.

    Arguments:      a, b   input vectors

    Returns:        distance
 *---------------------------------------------------------------------------*/
f32 VECDistance(const Vec* a, const Vec* b) {
    return sqrtf(VECSquareDistance(a, b));
}

/*---------------------------------------------------------------------------*
    Name:           VECNormalize

    Description:    Normalizes src into dst.

    Arguments:      src    input vector
                    dst    output normalized vector

    Returns:        none
 *---------------------------------------------------------------------------*/
void VECNormalize(const Vec* src, Vec* dst) {
    f32 mag = sqrtf(src->x * src->x + src->y * src->y + src->z * src->z);
    if (mag > 0.0f) {
        f32 inv = 1.0f / mag;
        dst->x = src->x * inv;
        dst->y = src->y * inv;
        dst->z = src->z * inv;
    } else {
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
}

/*---------------------------------------------------------------------------*
    Name:           VECAdd

    Description:    Adds vectors a and b into ab.

    Arguments:      a, b   input vectors
                    ab     output vector

    Returns:        none
 *---------------------------------------------------------------------------*/
void VECAdd(const Vec* a, const Vec* b, Vec* ab) {
    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}

/*---------------------------------------------------------------------------*
    Name:           VECSubtract

    Description:    Subtracts b from a (a - b).

    Arguments:      a, b   input vectors
                    a_b    output vector

    Returns:        none
 *---------------------------------------------------------------------------*/
void VECSubtract(const Vec* a, const Vec* b, Vec* a_b) {
    a_b->x = a->x - b->x;
    a_b->y = a->y - b->y;
    a_b->z = a->z - b->z;
}

/*---------------------------------------------------------------------------*
    Name:           VECReflect

    Description:    Reflects incident vector about normal, returning unit vector.

    Arguments:      src     incident vector (toward surface)
                    normal  surface normal (away from surface)
                    dst     reflected unit vector

    Returns:        none
 *---------------------------------------------------------------------------*/
void VECReflect(const Vec* src, const Vec* normal, Vec* dst) {
    f32 nx = normal->x;
    f32 ny = normal->y;
    f32 nz = normal->z;
    f32 nLen = sqrtf(nx * nx + ny * ny + nz * nz);
    f32 rx, ry, rz;
    f32 rLen;

    if (nLen <= 0.0f) {
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
        return;
    }

    nx /= nLen;
    ny /= nLen;
    nz /= nLen;

    {
        f32 d = src->x * nx + src->y * ny + src->z * nz;
        rx = src->x - 2.0f * d * nx;
        ry = src->y - 2.0f * d * ny;
        rz = src->z - 2.0f * d * nz;
    }

    rLen = sqrtf(rx * rx + ry * ry + rz * rz);
    if (rLen > 0.0f) {
        f32 inv = 1.0f / rLen;
        dst->x = rx * inv;
        dst->y = ry * inv;
        dst->z = rz * inv;
    } else {
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
}

/*---------------------------------------------------------------------------*
    Name:           VECHalfAngle

    Description:    Computes unit half-angle vector between light/view vectors.

    Arguments:      a      light->surface vector
                    b      view->surface vector
                    half   output half-angle unit vector (surface->halfway dir)

    Returns:        none
 *---------------------------------------------------------------------------*/
void VECHalfAngle(const Vec* a, const Vec* b, Vec* half) {
    f32 ax = -a->x;
    f32 ay = -a->y;
    f32 az = -a->z;
    f32 bx = -b->x;
    f32 by = -b->y;
    f32 bz = -b->z;
    f32 len;

    len = sqrtf(ax * ax + ay * ay + az * az);
    if (len > 0.0f) {
        f32 inv = 1.0f / len;
        ax *= inv;
        ay *= inv;
        az *= inv;
    }

    len = sqrtf(bx * bx + by * by + bz * bz);
    if (len > 0.0f) {
        f32 inv = 1.0f / len;
        bx *= inv;
        by *= inv;
        bz *= inv;
    }

    half->x = ax + bx;
    half->y = ay + by;
    half->z = az + bz;

    len = sqrtf(half->x * half->x + half->y * half->y + half->z * half->z);
    if (len > 0.0f) {
        f32 inv = 1.0f / len;
        half->x *= inv;
        half->y *= inv;
        half->z *= inv;
    } else {
        half->x = 0.0f;
        half->y = 0.0f;
        half->z = 0.0f;
    }
}

/*---------------------------------------------------------------------------*
    Name:           VECScale

    Description:    Scales src by scale into dst.

    Arguments:      src    input vector
                    dst    output vector
                    scale  scalar

    Returns:        none
 *---------------------------------------------------------------------------*/
void VECScale(const Vec* src, Vec* dst, f32 scale) {
    dst->x = src->x * scale;
    dst->y = src->y * scale;
    dst->z = src->z * scale;
}

/*---------------------------------------------------------------------------*
    Name:           MTXFrustum
    
    Description:    Creates a perspective projection matrix (4x4).
    
    Arguments:      m      output matrix (Mtx44)
                    t      top clipping plane
                    b      bottom clipping plane
                    l      left clipping plane
                    r      right clipping plane
                    n      near clipping plane
                    f      far clipping plane
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXFrustum(Mtx44 m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f) {
    f32 tmp;
    
    tmp = 1.0f / (r - l);
    m[0][0] = (2.0f * n * tmp);
    m[0][1] = 0.0f;
    m[0][2] = (tmp * (r + l));
    m[0][3] = 0.0f;
    
    tmp = 1.0f / (t - b);
    m[1][0] = 0.0f;
    m[1][1] = (2.0f * n * tmp);
    m[1][2] = (tmp * (t + b));
    m[1][3] = 0.0f;
    
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    tmp = 1.0f / (f - n);
    m[2][2] = (-n * tmp);
    m[2][3] = (tmp * -(f * n));
    
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = -1.0f;
    m[3][3] = 0.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXLookAt
    
    Description:    Creates a view matrix looking from camPos toward objPt
                    with up vector camUp.
    
    Arguments:      m       output matrix
                    camPos  camera position
                    camUp   up vector
                    target  target point to look at
    
    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXLookAt(Mtx m, const Point3d* camPos, const Vec* camUp, const Point3d* target) {
    Vec look, right, up;
    f32 len;
    
    /* vLook points from target to camera (camera-space +Z axis). */
    look.x = camPos->x - target->x;
    look.y = camPos->y - target->y;
    look.z = camPos->z - target->z;
    
    /* Normalize look. */
    len = sqrtf(look.x * look.x + look.y * look.y + look.z * look.z);
    if (len > 0.0f) {
        look.x /= len;
        look.y /= len;
        look.z /= len;
    }
    
    /* vRight = camUp x vLook */
    right.x = camUp->y * look.z - camUp->z * look.y;
    right.y = camUp->z * look.x - camUp->x * look.z;
    right.z = camUp->x * look.y - camUp->y * look.x;
    
    /* Normalize right. */
    len = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
    if (len > 0.0f) {
        right.x /= len;
        right.y /= len;
        right.z /= len;
    }
    
    /* vUp = vLook x vRight */
    up.x = look.y * right.z - look.z * right.y;
    up.y = look.z * right.x - look.x * right.z;
    up.z = look.x * right.y - look.y * right.x;
    
    /* Build row-major view matrix. */
    m[0][0] = right.x;
    m[0][1] = right.y;
    m[0][2] = right.z;
    m[0][3] = -(right.x * camPos->x + right.y * camPos->y + right.z * camPos->z);
    
    m[1][0] = up.x;
    m[1][1] = up.y;
    m[1][2] = up.z;
    m[1][3] = -(up.x * camPos->x + up.y * camPos->y + up.z * camPos->z);
    
    m[2][0] = look.x;
    m[2][1] = look.y;
    m[2][2] = look.z;
    m[2][3] = -(look.x * camPos->x + look.y * camPos->y + look.z * camPos->z);
}

/*---------------------------------------------------------------------------*
    Name:           MTXPerspective

    Description:    Creates a perspective projection matrix (4x4).

    Arguments:      m      output matrix (Mtx44)
                    fovY   vertical field of view in degrees
                    aspect aspect ratio (width/height)
                    n      near clipping plane
                    f      far clipping plane

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXPerspective(Mtx44 m, f32 fovY, f32 aspect, f32 n, f32 f) {
    f32 angle = 0.5f * fovY * 0.017453293f;
    f32 cot   = 1.0f / tanf(angle);
    f32 tmp   = 1.0f / (f - n);

    m[0][0] = cot / aspect;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = cot;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = -n * tmp;
    m[2][3] = tmp * -(f * n);
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = -1.0f;
    m[3][3] = 0.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXOrtho

    Description:    Creates an orthographic projection matrix (4x4).

    Arguments:      m      output matrix (Mtx44)
                    top, bottom, left, right
                    nearZ, farZ  near and far clipping planes

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXOrtho(Mtx44 m, f32 top, f32 bottom, f32 left, f32 right, f32 nearZ, f32 farZ) {
    f32 tmp = 1.0f / (right - left);
    m[0][0] = 2.0f * tmp;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = tmp * -(right + left);
    tmp     = 1.0f / (top - bottom);
    m[1][0] = 0.0f;
    m[1][1] = 2.0f * tmp;
    m[1][2] = 0.0f;
    m[1][3] = tmp * -(top + bottom);
    tmp     = 1.0f / (farZ - nearZ);
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = -1.0f * tmp;
    m[2][3] = -farZ * tmp;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXLightFrustum

    Description:    Creates a 3x4 texture projection matrix for frustum.

    Arguments:      m       output matrix (Mtx 3x4)
                    t,b,l,r frustum planes
                    n       near plane
                    scaleS, scaleT, transS, transT  texture scale/translation

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXLightFrustum(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 scaleS, f32 scaleT, f32 transS, f32 transT) {
    f32 tmp;

    tmp = 1.0f / (r - l);
    m[0][0] = scaleS * (n * tmp);
    m[0][1] = 0.0f;
    m[0][2] = scaleS * (tmp * (r + l)) - transS;
    m[0][3] = 0.0f;

    tmp = 1.0f / (t - b);
    m[1][0] = 0.0f;
    m[1][1] = scaleT * (n * tmp);
    m[1][2] = scaleT * (tmp * (t + b)) - transT;
    m[1][3] = 0.0f;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = -1.0f;
    m[2][3] = 0.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXLightPerspective

    Description:    Creates a 3x4 texture projection matrix for perspective
                    (light/texgen).

    Arguments:      m       output matrix (Mtx 3x4)
                    fovY    vertical field of view in degrees
                    aspect  aspect ratio
                    scaleS, scaleT, transS, transT  texture scale/translation

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXLightPerspective(Mtx m, f32 fovY, f32 aspect, f32 scaleS, f32 scaleT, f32 transS, f32 transT) {
    f32 angle = 0.5f * fovY * 0.017453293f;
    f32 cot   = 1.0f / tanf(angle);

    m[0][0] = scaleS * (cot / aspect);
    m[0][1] = 0.0f;
    m[0][2] = -transS;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = cot * scaleT;
    m[1][2] = -transT;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = -1.0f;
    m[2][3] = 0.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXLightOrtho

    Description:    Creates a 3x4 texture projection matrix for orthographic
                    light/texgen projection.

    Arguments:      m       output matrix (Mtx 3x4)
                    t,b,l,r view volume extents
                    scaleS, scaleT, transS, transT  texture scale/translation

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXLightOrtho(Mtx m, f32 t, f32 b, f32 l, f32 r, f32 scaleS, f32 scaleT, f32 transS, f32 transT) {
    f32 tmp;

    tmp = 1.0f / (r - l);
    m[0][0] = scaleS * (2.0f * tmp);
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = scaleS * (tmp * -(r + l)) + transS;

    tmp = 1.0f / (t - b);
    m[1][0] = 0.0f;
    m[1][1] = scaleT * (2.0f * tmp);
    m[1][2] = 0.0f;
    m[1][3] = scaleT * (tmp * -(t + b)) + transT;

    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 0.0f;
    m[2][3] = 1.0f;
}

/*---------------------------------------------------------------------------*
    Name:           MTXReflect

    Description:    Creates a reflection matrix from a plane point/normal.

    Arguments:      m      output matrix
                    p      point on the reflection plane
                    n      plane normal

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXReflect(Mtx m, const Point3d* p, const Vec* n) {
    f32 nx = n->x;
    f32 ny = n->y;
    f32 nz = n->z;
    f32 len = sqrtf(nx * nx + ny * ny + nz * nz);
    f32 d;

    if (len <= 0.0f) {
        MTXIdentity(m);
        return;
    }

    nx /= len;
    ny /= len;
    nz /= len;

    d = -(nx * p->x + ny * p->y + nz * p->z);

    m[0][0] = 1.0f - 2.0f * nx * nx;
    m[0][1] = -2.0f * nx * ny;
    m[0][2] = -2.0f * nx * nz;
    m[0][3] = -2.0f * d * nx;

    m[1][0] = -2.0f * ny * nx;
    m[1][1] = 1.0f - 2.0f * ny * ny;
    m[1][2] = -2.0f * ny * nz;
    m[1][3] = -2.0f * d * ny;

    m[2][0] = -2.0f * nz * nx;
    m[2][1] = -2.0f * nz * ny;
    m[2][2] = 1.0f - 2.0f * nz * nz;
    m[2][3] = -2.0f * d * nz;
}

/* C_MTXLightPerspective alias for Pikmin/Revolution SDK compatibility */
void C_MTXLightPerspective(Mtx m, f32 fovY, f32 aspect, f32 scaleS, f32 scaleT, f32 transS, f32 transT) {
    MTXLightPerspective(m, fovY, aspect, scaleS, scaleT, transS, transT);
}

/* PSMTX* aliases (C implementations for PC; paired-singles on GCN) */
void PSMTXIdentity(Mtx m) { MTXIdentity(m); }
void PSMTXCopy(const Mtx src, Mtx dst) { MTXCopy(src, dst); }
void PSMTXConcat(const Mtx a, const Mtx b, Mtx ab) { MTXConcat(a, b, ab); }
void PSMTXMultVecArray(const Mtx m, const Vec* srcBase, Vec* dstBase, u32 count) { MTXMultVecArray(m, srcBase, dstBase, count); }

void PSMTXReorder(const Mtx src, ROMtx dest) {
    dest[0][0] = src[0][0];
    dest[0][1] = src[1][0];
    dest[0][2] = src[2][0];
    dest[1][0] = src[0][1];
    dest[1][1] = src[1][1];
    dest[1][2] = src[2][1];
    dest[2][0] = src[0][2];
    dest[2][1] = src[1][2];
    dest[2][2] = src[2][2];
    dest[3][0] = src[0][3];
    dest[3][1] = src[1][3];
    dest[3][2] = src[2][3];
}

void PSMTXROMultVecArray(const ROMtx m, const Vec* srcBase, Vec* dstBase, u32 count) {
    while (count--) {
        f32 x = srcBase->x;
        f32 y = srcBase->y;
        f32 z = srcBase->z;

        dstBase->x = m[0][0] * x + m[1][0] * y + m[2][0] * z + m[3][0];
        dstBase->y = m[0][1] * x + m[1][1] * y + m[2][1] * z + m[3][1];
        dstBase->z = m[0][2] * x + m[1][2] * y + m[2][2] * z + m[3][2];

        ++srcBase;
        ++dstBase;
    }
}

void PSMTXScale(Mtx m, f32 xS, f32 yS, f32 zS) {
    m[0][0] = xS;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = yS;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = zS;
    m[2][3] = 0.0f;
}

void PSMTXTranspose(const Mtx src, Mtx dst) {
    dst[0][0] = src[0][0];
    dst[0][1] = src[1][0];
    dst[0][2] = src[2][0];
    dst[0][3] = src[0][3];
    dst[1][0] = src[0][1];
    dst[1][1] = src[1][1];
    dst[1][2] = src[2][1];
    dst[1][3] = src[1][3];
    dst[2][0] = src[0][2];
    dst[2][1] = src[1][2];
    dst[2][2] = src[2][2];
    dst[2][3] = src[2][3];
}

u32 PSMTXInverse(const Mtx src, Mtx inv) {
    f32 det = src[0][0] * (src[1][1] * src[2][2] - src[1][2] * src[2][1])
            - src[0][1] * (src[1][0] * src[2][2] - src[1][2] * src[2][0])
            + src[0][2] * (src[1][0] * src[2][1] - src[1][1] * src[2][0]);
    if (det * det < 1e-10f) return 0;
    f32 rdet = 1.0f / det;
    inv[0][0] =  (src[1][1] * src[2][2] - src[1][2] * src[2][1]) * rdet;
    inv[0][1] = -(src[0][1] * src[2][2] - src[0][2] * src[2][1]) * rdet;
    inv[0][2] =  (src[0][1] * src[1][2] - src[0][2] * src[1][1]) * rdet;
    inv[0][3] = -(inv[0][0] * src[0][3] + inv[0][1] * src[1][3] + inv[0][2] * src[2][3]);
    inv[1][0] = -(src[1][0] * src[2][2] - src[1][2] * src[2][0]) * rdet;
    inv[1][1] =  (src[0][0] * src[2][2] - src[0][2] * src[2][0]) * rdet;
    inv[1][2] = -(src[0][0] * src[1][2] - src[0][2] * src[1][0]) * rdet;
    inv[1][3] = -(inv[1][0] * src[0][3] + inv[1][1] * src[1][3] + inv[1][2] * src[2][3]);
    inv[2][0] =  (src[1][0] * src[2][1] - src[1][1] * src[2][0]) * rdet;
    inv[2][1] = -(src[0][0] * src[2][1] - src[0][1] * src[2][0]) * rdet;
    inv[2][2] =  (src[0][0] * src[1][1] - src[0][1] * src[1][0]) * rdet;
    inv[2][3] = -(inv[2][0] * src[0][3] + inv[2][1] * src[1][3] + inv[2][2] * src[2][3]);
    return 1;
}

void MTXTranspose(const Mtx src, Mtx dst) {
    PSMTXTranspose(src, dst);
}

u32 MTXInverse(const Mtx src, Mtx inv) {
    return PSMTXInverse(src, inv);
}

/* PSVECMag: declared in Pikmin Dolphin/vec.h; avoid duplicate in mtx.h */
f32 PSVECMag(const Vec* v) {
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}
