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
    Vec f, r, u;
    f32 len;

    /* f = camPos - target: points FROM target TOWARD camera.
       In GX right-handed view space, this is the +Z axis (camera looks down -Z).
       Objects in front of the camera will have negative view-space Z. */
    f.x = camPos->x - target->x;
    f.y = camPos->y - target->y;
    f.z = camPos->z - target->z;

    len = sqrtf(f.x * f.x + f.y * f.y + f.z * f.z);
    if (len > 0.0f) {
        f.x /= len;
        f.y /= len;
        f.z /= len;
    }

    /* r = camUp x f  (right-hand rule: right = up cross backward) */
    r.x = camUp->y * f.z - camUp->z * f.y;
    r.y = camUp->z * f.x - camUp->x * f.z;
    r.z = camUp->x * f.y - camUp->y * f.x;

    len = sqrtf(r.x * r.x + r.y * r.y + r.z * r.z);
    if (len > 0.0f) {
        r.x /= len;
        r.y /= len;
        r.z /= len;
    }

    /* u = f x r  (re-orthogonalised up) */
    u.x = f.y * r.z - f.z * r.y;
    u.y = f.z * r.x - f.x * r.z;
    u.z = f.x * r.y - f.y * r.x;

    /* Store each basis vector in a ROW (standard column-vector M*v convention).
       Translation = -dot(basis, camPos) to move world into view space. */
    m[0][0] = r.x;  m[0][1] = r.y;  m[0][2] = r.z;
    m[0][3] = -(r.x * camPos->x + r.y * camPos->y + r.z * camPos->z);

    m[1][0] = u.x;  m[1][1] = u.y;  m[1][2] = u.z;
    m[1][3] = -(u.x * camPos->x + u.y * camPos->y + u.z * camPos->z);

    m[2][0] = f.x;  m[2][1] = f.y;  m[2][2] = f.z;
    m[2][3] = -(f.x * camPos->x + f.y * camPos->y + f.z * camPos->z);
}

