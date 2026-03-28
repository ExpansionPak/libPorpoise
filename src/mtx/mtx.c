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
    Name:           MTXRotDeg

    Description:    Creates a rotation matrix from axis and angle in degrees.

    Arguments:      m      output matrix
                    axis   'x','y','z' (case-insensitive)
                    deg    angle in degrees

    Returns:        none
 *---------------------------------------------------------------------------*/
void MTXRotDeg(Mtx m, char axis, f32 deg) {
    f32 rad = MTXDegToRad(deg);
    f32 s = sinf(rad);
    f32 c = cosf(rad);

    MTXIdentity(m);
    switch (axis) {
        case 'x':
        case 'X':
            m[1][1] = c;
            m[1][2] = -s;
            m[2][1] = s;
            m[2][2] = c;
            break;
        case 'y':
        case 'Y':
            m[0][0] = c;
            m[0][2] = s;
            m[2][0] = -s;
            m[2][2] = c;
            break;
        case 'z':
        case 'Z':
            m[0][0] = c;
            m[0][1] = -s;
            m[1][0] = s;
            m[1][1] = c;
            break;
        default:
            break;
    }
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
