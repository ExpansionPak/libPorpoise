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
    Vec forward, right, up;
    f32 len;
    
    // Calculate forward vector (from camera to target)
    forward.x = target->x - camPos->x;
    forward.y = target->y - camPos->y;
    forward.z = target->z - camPos->z;
    
    // Normalize forward
    len = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (len > 0.0f) {
        forward.x /= len;
        forward.y /= len;
        forward.z /= len;
    }
    
    // Calculate right vector (forward cross up)
    right.x = forward.y * camUp->z - forward.z * camUp->y;
    right.y = forward.z * camUp->x - forward.x * camUp->z;
    right.z = forward.x * camUp->y - forward.y * camUp->x;
    
    // Normalize right
    len = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
    if (len > 0.0f) {
        right.x /= len;
        right.y /= len;
        right.z /= len;
    }
    
    // Calculate up vector (right cross forward)
    up.x = right.y * forward.z - right.z * forward.y;
    up.y = right.z * forward.x - right.x * forward.z;
    up.z = right.x * forward.y - right.y * forward.x;
    
    // Build matrix (inverse of camera transform)
    m[0][0] = right.x;
    m[0][1] = up.x;
    m[0][2] = -forward.x;
    m[0][3] = -(right.x * camPos->x + up.x * camPos->y - forward.x * camPos->z);
    
    m[1][0] = right.y;
    m[1][1] = up.y;
    m[1][2] = -forward.y;
    m[1][3] = -(right.y * camPos->x + up.y * camPos->y - forward.y * camPos->z);
    
    m[2][0] = right.z;
    m[2][1] = up.z;
    m[2][2] = -forward.z;
    m[2][3] = -(right.z * camPos->x + up.z * camPos->y - forward.z * camPos->z);
}

