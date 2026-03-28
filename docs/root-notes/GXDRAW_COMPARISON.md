# GXDraw.c Comparison: Reference vs Our Implementation

## Overview

This document compares the reference Nintendo GXDraw.c implementation (`reference/gx/GXDraw.c`) with our current implementation (`src/gx/GXDraw.c`).

---

## Major Structural Differences

### 1. **State Management**

**Reference:**
```c
static GXVtxDescList    vcd[GX_MAX_VTXDESCLIST_SZ];
static GXVtxAttrFmtList vat[GX_MAX_VTXATTRFMTLIST_SZ];
```

**Our Implementation:**
```c
static u32 gSavedVCD[54];    // Vertex descriptor backup
static u32 gSavedVAT[72];    // Vertex attribute format backup
```

**Issue:** We use raw arrays instead of proper GX structure types. This may cause compatibility issues.

---

### 2. **Display List Support**

**Reference:** 
- No display list support
- Calls `GXPosition3f32()` and `GXNormal3f32()` directly

**Our Implementation:**
- Has display list support via `gDisplayListActive` and `gDisplayListBufferPtr`
- Uses wrapper functions `SubmitPosition3f32()`, `SubmitNormal3f32()`, etc.
- Checks if display list is active before calling EmGX functions

**Note:** This is an enhancement, but may not match original SDK behavior.

---

### 3. **Function Signatures**

| Function | Reference | Our Implementation | Difference |
|----------|-----------|-------------------|------------|
| `GXDrawCylinder` | `(u8 numEdges)` | `(u8 numSegments)` | Parameter name only |
| `GXDrawTorus` | `(f32 rc, u8 numc, u8 numt)` | `(f32 innerRadius, u8 numRings, u8 numSides)` | Different parameter names |
| `GXDrawSphere` | `(u8 numMajor, u8 numMinor)` | `(u8 numStacks, u8 numSlices)` | Different parameter names |
| `GXDrawCube` | `(void)` | `(bool useVertexNormals, bool useTexCoords)` | **MAJOR: Different API** |

**Issue:** `GXDrawCube` has completely different signatures. Reference version checks vertex descriptors internally, ours takes parameters.

---

### 4. **Type Usage**

**Reference:**
- Uses `GXAttrType` for attribute type checking
- Uses `GX_NONE` constant (value `0`) to check if attributes are enabled
- Uses `GX_NRM_XYZ` (value `0`) for normal format
- Uses `GX_TEX_ST` (value `1`) for texture coordinate format
- Uses `GX_S8` (value `1`) for signed 8-bit format
- Uses `GX_VA_NBT` for binormal/tangent attributes
- Uses `GX_NRM_NBT` for NBT normal format

**Our Implementation:**
- Uses `u32` for attribute type checking
- Doesn't check for `GX_NONE` (may cause issues)
- Uses `0` for normal format ✅ (correct, matches `GX_NRM_XYZ`)
- Uses `GX_POS_XYZ` for texture coords ❌ (should be `GX_TEX_ST` which is `1`)
- Uses `1` for texture format ✅ (correct, matches `GX_S8`)
- Uses `GX_VA_CLR0` for binormals ❌ (should be `GX_VA_NBT`)

**Issues:** Multiple incorrect constant values that may cause rendering problems.

---

### 5. **Dodecahedron Vertex Data**

**Reference:**
```c
#define A  (0.5F * 1.61803F)  /* (sqrt(5) + 1) / 2 */
#define B  (0.5F * 0.61803F)  /* (sqrt(5) - 1) / 2 */
#define C  (0.5F * 1.0F)
static f32 verts[20][3] = {
    {-A, 0.0F, B},
    {-A, 0.0F, -B},
    // ... uses golden ratio constants
};
```

**Our Implementation:**
```c
static const f32 dodecaVerts[20][3] = {
    {-0.809017f,  0.000000f,  0.309017f},
    {-0.809017f,  0.000000f, -0.309017f},
    // ... hardcoded values
};
```

**Note:** Values appear equivalent (0.809017 ≈ 0.5 * 1.61803, 0.309017 ≈ 0.5 * 0.61803), but reference uses symbolic constants.

---

### 6. **Error Checking**

**Reference:**
```c
ASSERTMSG(numEdges <= 99, GXERR_DRAW_CYLINDER_EDGES);
ASSERTMSG(rc < 1.0F, GXERR_DRAW_TORUS_FAT);
ASSERTMSG(d != 0.0F, GXERR_DRAW_NORMALIZE_ZEROLENVECTOR);
```

**Our Implementation:**
- No error checking or assertions

**Issue:** Missing validation could lead to crashes or incorrect rendering.

---

### 7. **Vector Math Functions**

**Reference:**
- `vsub(p1, p2, u)` - subtracts p1 from p2: `u = p2 - p1`
- `vcross(u, v, n)` - cross product
- `normalize(v)` - normalizes in place with assertion check

**Our Implementation:**
- `VecSub(a, b, out)` - subtracts a from b: `out = b - a` (same logic, different order)
- `VecCross(a, b, out)` - cross product (same)
- `VecNormalize(v)` - normalizes in place (no assertion)

**Note:** Logic is equivalent, but reference has better error checking.

---

### 8. **Cube Face Drawing**

**Reference:**
```c
static void GXDrawCubeFace(f32 nx, f32 ny, f32 nz,
                           f32 tx, f32 ty, f32 tz,  // tangent
                           f32 bx, f32 by, f32 bz,  // binormal
                           GXAttrType binormal, GXAttrType texture)
{
    const f32 SZ = 1.0F / SQRT3;  // Uses SQRT3 constant
    
    // Draws 4 vertices with conditional binormal/texture
    GXPosition3f32((nx+tx+bx)*SZ, (ny+ty+by)*SZ, (nz+tz+bz)*SZ);
    GXNormal3f32(nx, ny, nz);
    if (binormal != GX_NONE) {
        GXNormal3f32(tx, ty, tz);
        GXNormal3f32(bx, by, bz);
    }
    if (texture != GX_NONE) {
        GXTexCoord2s8(1, 1);
    }
    // ... 3 more vertices
}
```

**Our Implementation:**
```c
static void GXDrawCubeFace(f32 nx, f32 ny, f32 nz,
                           f32 ux, f32 uy, f32 uz,  // "up"
                           f32 rx, f32 ry, f32 rz,  // "right"
                           bool hasNormals, bool hasTexCoords)
{
    // Computes corners differently
    // Uses Vec3 arrays and loops
    // Different vertex order
}
```

**Issues:**
- Different parameter semantics (tangent/binormal vs up/right)
- Different vertex computation
- Different constant (INV_SQRT3 vs 1.0F/SQRT3, but equivalent)
- Uses bool instead of GXAttrType

---

### 9. **Normal Table Generation**

**Reference:**
```c
static u32 CmpNormal32(f32* n1, f32* n2) {
    u32 i;
    for (i = 0; i < 3; i++)
        if (n1[i] != n2[i])
            return 0;
    return 1;
}
```

**Our Implementation:**
```c
static bool VecEqual(const Vec3 a, const Vec3 b) {
    return (a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]);
}
```

**Note:** Logic is equivalent, but reference uses exact float comparison (may have precision issues).

---

### 10. **Subdivision Order**

**Reference:**
```c
Subdivide(depth-1, v0,  v01, v20);
Subdivide(depth-1, v1,  v12, v01);
Subdivide(depth-1, v2,  v20, v12);
Subdivide(depth-1, v01, v12, v20);
```

**Our Implementation:**
```c
Subdivide(depth - 1, v0,  v01, v20);
Subdivide(depth - 1, v01, v1,  v12);
Subdivide(depth - 1, v20, v12, v2);
Subdivide(depth - 1, v01, v12, v20);
```

**Issue:** Different vertex order in second and third recursive calls. This may produce different triangle winding or ordering.

---

## Summary of Issues

### Critical Issues (Must Fix)
1. ❌ **Wrong constant values**: `GX_NRM_XYZ`, `GX_TEX_ST`, `GX_S8`, `GX_VA_NBT`
2. ❌ **Missing `GX_NONE` checks**: May enable attributes that shouldn't be enabled
3. ❌ **`GXDrawCube` API mismatch**: Completely different function signature
4. ❌ **Wrong state structure types**: Should use `GXVtxDescList` and `GXVtxAttrFmtList`

### Moderate Issues (Should Fix)
5. ⚠️ **Missing error checking**: No assertions for invalid parameters
6. ⚠️ **Subdivision order difference**: May affect triangle ordering
7. ⚠️ **Cube face computation**: Different algorithm may produce different results

### Minor Issues (Nice to Have)
8. ℹ️ **Display list support**: Enhancement, but may not match original behavior
9. ℹ️ **Hardcoded constants**: Reference uses symbolic constants (golden ratio)
10. ℹ️ **Function parameter names**: Different but functionally equivalent

---

## Recommendations

1. **Add proper GX type definitions** for `GXVtxDescList`, `GXVtxAttrFmtList`, `GXAttrType`
2. **Fix constant values** to match reference (`GX_NRM_XYZ`, `GX_TEX_ST`, etc.)
3. **Add `GX_NONE` checks** before enabling optional attributes
4. **Align `GXDrawCube` signature** with reference (or document the difference)
5. **Add error checking** with assertions or return codes
6. **Fix subdivision order** to match reference exactly
7. **Review cube face computation** to ensure it matches reference behavior

---

## Files to Check

- `reference/gx/GXEnum.h` - For constant definitions
- `reference/gx/GXStruct.h` - For structure definitions
- `reference/gx/GXAssert.h` - For assertion macros

