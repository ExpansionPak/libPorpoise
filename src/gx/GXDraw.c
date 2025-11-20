/*---------------------------------------------------------------------------*
  GXDraw.c - GX Procedural Geometry Drawing
  
  Based on reverse-engineered Dolphin emulator GX implementation.
  Recreates the original GameCube/Wii GX drawing functions.
  
  Source: c:\rabin\dolphin\build\libraries\gx\src\gxdraw.c
 *---------------------------------------------------------------------------*/

#include <dolphin/gx/GXDraw.h>
#include <dolphin/gx/GXEnum.h>
#include <dolphin/gx/GXStruct.h>
#include <dolphin/os.h>
#include <math.h>

#define PI    3.14159265358979323846F
#define SQRT3 1.732050808F

/*---------------------------------------------------------------------------*
    Static State
 *---------------------------------------------------------------------------*/

/* Vertex descriptor state for saving/restoring */
static GXVtxDescList    vcd[GX_MAX_VTXDESCLIST_SZ];
static GXVtxAttrFmtList vat[GX_MAX_VTXATTRFMTLIST_SZ];

/* Normal table generation state */
static u32  nrm_cnt;
static f32* nrm_tab;

/*---------------------------------------------------------------------------*
    External Function Declarations
 *---------------------------------------------------------------------------*/

/* GX API functions - implemented in other modules */
extern void GXGetVtxDescv(void* vcd);
extern void GXGetVtxAttrFmtv(GXVtxFmt vtxfmt, void* vat);
extern void GXClearVtxDesc(void);
extern void GXSetVtxDesc(GXAttr attr, GXAttrType type);
extern void GXSetVtxAttrFmt(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType comp, u8 shift);
extern void GXGetVtxDesc(GXAttr attr, GXAttrType* type);
extern void GXSetVtxDescv(const void* vcd);
extern void GXSetVtxAttrFmtv(GXVtxFmt vtxfmt, const void* vat);
extern void GXBegin(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);
extern void GXEnd(void);
extern void GXPosition3f32(f32 x, f32 y, f32 z);
extern void GXNormal3f32(f32 x, f32 y, f32 z);
extern void GXTexCoord2f32(f32 s, f32 t);
extern void GXTexCoord2s8(s8 s, s8 t);

/* Math functions */
extern double cos(double x);
extern double sin(double x);
extern double sqrt(double x);

/* OS functions */
extern void OSPanic(const char* file, s32 line, const char* msg);

/*---------------------------------------------------------------------------*
    Helper Functions (Internal to this module)
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * Save current vertex state and set up for procedural drawing
 *---------------------------------------------------------------------------*/
static void GetVertState(void)
{
    GXGetVtxDescv(vcd);
    GXGetVtxAttrFmtv(GX_VTXFMT3, vat);
    
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
}

/*---------------------------------------------------------------------------*
 * Restore previous vertex state
 *---------------------------------------------------------------------------*/
static void RestoreVertState(void)
{
    GXSetVtxDescv(vcd);
    GXSetVtxAttrFmtv(GX_VTXFMT3, vat);
}

/*---------------------------------------------------------------------------*
 * Vector subtraction: u = p2 - p1
 *---------------------------------------------------------------------------*/
static void vsub(f32 *p1, f32 *p2, f32 *u)
{
    u32 i;
    for (i = 0; i < 3; i++)
        u[i] = p2[i] - p1[i];
}

/*---------------------------------------------------------------------------*
 * Vector cross product: n = u x v
 *---------------------------------------------------------------------------*/
static void vcross(f32 *u, f32 *v, f32 *n)
{
    f32 n1[3];

    n1[0] = u[1] * v[2] - u[2] * v[1];
    n1[1] = u[2] * v[0] - u[0] * v[2];
    n1[2] = u[0] * v[1] - u[1] * v[0];
    n[0] = n1[0];
    n[1] = n1[1];
    n[2] = n1[2];
}

/*---------------------------------------------------------------------------*
 * Normalize vector v
 *---------------------------------------------------------------------------*/
static void normalize(f32 *v)
{
    f32 d = (f32)sqrt((double)(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]));
    if (d == 0.0F) {
        OSPanic("GXDraw.c", 87, "normalize: zero length vector");
        return;
    }
    v[0] /= d; v[1] /= d; v[2] /= d;
}

/*---------------------------------------------------------------------------*
 * Helper to emit vertex with position and normal
 *---------------------------------------------------------------------------*/
static void myvertex(f32 *v, f32 *n)
{
    GXPosition3f32(v[0], v[1], v[2]);
    GXNormal3f32(n[0], n[1], n[2]);
}

/*---------------------------------------------------------------------------*
 * Draw a triangle from three vertices
 *---------------------------------------------------------------------------*/
static void DumpTriangle(f32 *v0, f32 *v1, f32 *v2)
{
    GXBegin(GX_TRIANGLES, GX_VTXFMT3, 3);
        myvertex(v0, v0);
        myvertex(v1, v1);
        myvertex(v2, v2);
    GXEnd();
}

/*---------------------------------------------------------------------------*
 * Recursive triangle subdivision for sphere generation
 *---------------------------------------------------------------------------*/
static void Subdivide(u8 depth, f32 *v0, f32 *v1, f32 *v2)
{
    f32 v01[3], v12[3], v20[3];
    u32 i;

    if(depth == 0) {
        DumpTriangle(v0, v1, v2);
        return;
    }
    for (i = 0; i < 3; i++) {
        v01[i] = v0[i] + v1[i];
        v12[i] = v1[i] + v2[i];
        v20[i] = v2[i] + v0[i];
    }
    normalize(v01);
    normalize(v12);
    normalize(v20);
    Subdivide((u8)(depth-1), v0, v01, v20);
    Subdivide((u8)(depth-1), v1, v12, v01);
    Subdivide((u8)(depth-1), v2, v20, v12);
    Subdivide((u8)(depth-1), v01, v12, v20);
}

/*---------------------------------------------------------------------------*
 * Subdivide a triangle from indexed data
 *---------------------------------------------------------------------------*/
static void SubDivTriangle(u8 depth, u8 i, f32 data[][3], u8 ndx[][3])
{
  f32 *x0, *x1, *x2;

  x0 = data[ndx[i][0]];
  x1 = data[ndx[i][1]];
  x2 = data[ndx[i][2]];
  Subdivide(depth, x0, x1, x2);
}

/*---------------------------------------------------------------------------*
 * Compare two normals (for normal table generation)
 *---------------------------------------------------------------------------*/
static u32 CmpNormal32(f32* n1, f32* n2)
{
    u32 i;
    for (i = 0; i < 3; i++)
    if (n1[i] != n2[i])
        return 0;
    return 1;
}

/*---------------------------------------------------------------------------*
 * Add normal to table if not already present
 *---------------------------------------------------------------------------*/
static void AddNormal(f32 *n) 
{
    u32 indx, i;

    // compare normal to all normals in table
    // if normal not in table, add it
    for (i = 0; i < nrm_cnt; i++)
    {
        if (CmpNormal32(n, &nrm_tab[i*3]))
        return;
    }
    // if normal not in table, add it
    indx = nrm_cnt*3;
    nrm_tab[indx]   = n[0];
    nrm_tab[indx+1] = n[1];
    nrm_tab[indx+2] = n[2];

    nrm_cnt++;
}

/*---------------------------------------------------------------------------*
 * Recursive subdivision for normal table generation
 *---------------------------------------------------------------------------*/
static void SubdivideNrm(u8 depth, f32* v0, f32* v1, f32* v2)
{
    f32 v01[3], v12[3], v20[3];
    u32 i;

    if(depth == 0) {
        AddNormal(v0);
        AddNormal(v1);
        AddNormal(v2);
        return;
    }
    for (i = 0; i < 3; i++) {
        v01[i] = v0[i] + v1[i];
        v12[i] = v1[i] + v2[i];
        v20[i] = v2[i] + v0[i];
    }
    normalize(v01);
    normalize(v12);
    normalize(v20);
    SubdivideNrm((u8)(depth-1), v0, v01, v20);
    SubdivideNrm((u8)(depth-1), v1, v12, v01);
    SubdivideNrm((u8)(depth-1), v2, v20, v12);
    SubdivideNrm((u8)(depth-1), v01, v12, v20);
}

/*---------------------------------------------------------------------------*
 * Subdivide triangle for normal table generation
 *---------------------------------------------------------------------------*/
static void SubDivNrm(u8 depth, u8 i, f32 data[][3], u8 ndx[][3])
{
    f32 *x0, *x1, *x2;

    x0 = data[ndx[i][0]];
    x1 = data[ndx[i][1]];
    x2 = data[ndx[i][2]];
    SubdivideNrm(depth, x0, x1, x2);
}

/*---------------------------------------------------------------------------*
 * Draw one face of a cube
 *---------------------------------------------------------------------------*/
static void GXDrawCubeFace(f32 nx, f32 ny, f32 nz,
                             f32 tx, f32 ty, f32 tz,
                             f32 bx, f32 by, f32 bz,
                             GXAttrType binormal, GXAttrType texture)
{
    const f32 SZ = 1.0F / SQRT3;
    
    GXPosition3f32((nx+tx+bx)*SZ, (ny+ty+by)*SZ, (nz+tz+bz)*SZ);
    GXNormal3f32(nx, ny, nz);
    if (binormal != GX_NONE)
    {
        GXNormal3f32(tx, ty, tz);
        GXNormal3f32(bx, by, bz);
    }
    if (texture != GX_NONE)
    {
        GXTexCoord2s8(1, 1);
    }
    GXPosition3f32((nx-tx+bx)*SZ, (ny-ty+by)*SZ, (nz-tz+bz)*SZ);
    GXNormal3f32(nx, ny, nz);
    if (binormal != GX_NONE)
    {
        GXNormal3f32(tx, ty, tz);
        GXNormal3f32(bx, by, bz);
    }
    if (texture != GX_NONE)
    {
        GXTexCoord2s8(0, 1);
    }
    GXPosition3f32((nx-tx-bx)*SZ, (ny-ty-by)*SZ, (nz-tz-bz)*SZ);
    GXNormal3f32(nx, ny, nz);
    if (binormal != GX_NONE)
    {
        GXNormal3f32(tx, ty, tz);
        GXNormal3f32(bx, by, bz);
    }
    if (texture != GX_NONE)
    {
        GXTexCoord2s8(0, 0);
    }
    GXPosition3f32((nx+tx-bx)*SZ, (ny+ty-by)*SZ, (nz+tz-bz)*SZ);
    GXNormal3f32(nx, ny, nz);
    if (binormal != GX_NONE)
    {
        GXNormal3f32(tx, ty, tz);
        GXNormal3f32(bx, by, bz);
    }
    if (texture != GX_NONE)
    {
        GXTexCoord2s8(1, 0);
    }
}

/*---------------------------------------------------------------------------*
    Static Data
 *---------------------------------------------------------------------------*/

/* Dodecahedron data */
static u32 polygons[12][5] =
{
        {0, 12, 10, 11, 16},
        {1, 17, 8, 9, 13},
        {2, 14, 9, 8, 18},
        {3, 19, 11, 10, 15},
        {4, 14, 2, 3, 15},
        {5, 12, 0, 1, 13},
        {6, 17, 1, 0, 16},
        {7, 19, 3, 2, 18},
        {8, 17, 6, 7, 18},
        {9, 14, 4, 5, 13},
        {10, 12, 5, 4, 15},
        {11, 19, 7, 6, 16},
};

#define A  (0.5F * 1.61803F)  /* (sqrt(5) + 1) / 2 */
#define B  (0.5F * 0.61803F)  /* (sqrt(5) - 1) / 2 */
#define C  (0.5F * 1.0F)
static f32 verts[20][3] =
{
        {-A, 0.0F, B},
        {-A, 0.0F, -B},
        {A, 0.0F, -B},
        {A, 0.0F, B},
        {B, -A, 0.0F},
        {-B, -A, 0.0F},
        {-B, A, 0.0F},
        {B, A, 0.0F},
        {0.0F, B, -A},
        {0.0F, -B, -A},
        {0.0F, -B, A},
        {0.0F, B, A},
        {-C, -C, C},
        {-C, -C, -C},
        {C, -C, -C},
        {C, -C, C},
        {-C, C, C},
        {-C, C, -C},
        {C, C, -C},
        {C, C, C},
};
#undef A
#undef B
#undef C

/* Octahedron data */
static f32 odata[6][3] =
{
  {1.0F, 0.0F, 0.0F},
  {-1.0F, 0.0F, 0.0F},
  {0.0F, 1.0F, 0.0F},
  {0.0F, -1.0F, 0.0F},
  {0.0F, 0.0F, 1.0F},
  {0.0F, 0.0F, -1.0F}
};

static u8 ondex[8][3] =
{
  {0, 4, 2},
  {1, 2, 4},
  {0, 3, 4},
  {1, 4, 3},
  {0, 2, 5},
  {1, 5, 2},
  {0, 5, 3},
  {1, 3, 5}
};

/* Icosahedron data */
#define X 0.525731112119133606F
#define Z 0.850650808352039932F

static f32 idata[12][3] =
{
  {-X, 0.0F, Z},
  {X, 0.0F, Z},
  {-X, 0.0F, -Z},
  {X, 0.0F, -Z},
  {0.0F, Z, X},
  {0.0F, Z, -X},
  {0.0F, -Z, X},
  {0.0F, -Z, -X},
  {Z, X, 0.0F},
  {-Z, X, 0.0F},
  {Z, -X, 0.0F},
  {-Z, -X, 0.0F}
};

#undef X

static u8 index[20][3] =
{
  {0, 4, 1},
  {0, 9, 4},
  {9, 5, 4},
  {4, 5, 8},
  {4, 8, 1},
  {8, 10, 1},
  {8, 3, 10},
  {5, 3, 8},
  {5, 2, 3},
  {2, 7, 3},
  {7, 10, 3},
  {7, 6, 10},
  {7, 11, 6},
  {11, 0, 6},
  {0, 1, 6},
  {6, 1, 10},
  {9, 0, 11},
  {9, 11, 2},
  {9, 2, 5},
  {7, 2, 11},
};

/*---------------------------------------------------------------------------*
    Public API Functions
 *---------------------------------------------------------------------------*/

void GXDrawCylinder(u8 numEdges)
{
    s32              i;
    f32              top = 1.0F;
    f32              bottom = -top;
    f32              x[100];
    f32              y[100];
    f32              angle; 
    
    if (numEdges > 99) {
        OSPanic("GXDraw.c", 99, "GXDrawCylinder: too many edges");
        return;
    }

    GetVertState();

    for (i = 0; i <= numEdges; i++) 
    {
        angle = (f32)((i * 2.0 * PI) / numEdges);
        x[i] = (f32)cos((double)angle);
        y[i] = (f32)sin((double)angle);
    }

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT3, (u16)((numEdges+1)*2));
    for (i = 0; i <= numEdges; i++) 
    {
        GXPosition3f32(x[i], y[i], bottom);
        GXNormal3f32(x[i], y[i], 0.0F);
        GXPosition3f32(x[i], y[i], top);
        GXNormal3f32(x[i], y[i], 0.0F);
    }
    GXEnd();

    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT3, (u16)((numEdges+2)));
        GXPosition3f32(0.0F, 0.0F, top);
        GXNormal3f32(0.0F, 0.0F, 1.0F);
        for (i = 0; i <= numEdges; i++) 
        {
            GXPosition3f32(x[i], -y[i], top);
            GXNormal3f32(0.0F, 0.0F, 1.0F);
        }
    GXEnd();

    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT3, (u16)((numEdges+2)));
        GXPosition3f32(0.0F, 0.0F, bottom);
        GXNormal3f32(0.0F, 0.0F, -1.0F);
        for (i = 0; i <= numEdges; i++) 
        {
            GXPosition3f32(x[i], y[i], bottom);
            GXNormal3f32(0.0F, 0.0F, -1.0F);
        }
    GXEnd();

    RestoreVertState();
}

void GXDrawTorus(f32 rc, u8 numc, u8 numt)
{
    GXAttrType ttype;
    s32 i, j, k;
    f32 s, t;
    f32 x, y, z;
    f32 twopi = 2.0F * PI;
    f32 rt;

    if (rc >= 1.0F) {
        OSPanic("GXDraw.c", 134, "GXDrawTorus: doughnut too fat");
        return;
    }

    rt = 1.0F - rc;
    GXGetVtxDesc(GX_VA_TEX0, &ttype);
    GetVertState();
    if (ttype != GX_NONE)
    {
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    }

    for (i = 0; i < numc; i++) {
      GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT3, (u16)((numt+1)*2));
        for (j = 0; j <= numt; j++) {
            for (k = 1; k >= 0; k--) {
                s = (i + k) % numc;
                t = (f32)(j % numt);

                x = (rt - rc * (f32)cos((double)(s*twopi/numc))) * (f32)cos((double)(t*twopi/numt));
                y = (rt - rc * (f32)cos((double)(s*twopi/numc))) * (f32)sin((double)(t*twopi/numt));
                z = rc * (f32)sin((double)(s*twopi/numc));
                GXPosition3f32(x, y, z);

                x = -(f32)cos((double)(t*twopi/numt)) * (f32)cos((double)(s*twopi/numc));
                y = -(f32)sin((double)(t*twopi/numt)) * (f32)cos((double)(s*twopi/numc));
                z =  (f32)sin((double)(s*twopi/numc));
                GXNormal3f32(x, y, z);

                if (ttype != GX_NONE)
                {
                    GXTexCoord2f32((f32)(i+k)/numc, (f32)j/numt);
                }
            }
        }
      GXEnd();
    }

    RestoreVertState();
}

void GXDrawSphere(u8 numMajor, u8 numMinor)
{
  GXAttrType ttype;
  f32 radius = 1.0F;
  f32 majorStep = (PI / numMajor);
  f32 minorStep = (2.0F * PI / numMinor);
  s32 i, j;

  GXGetVtxDesc(GX_VA_TEX0, &ttype);
  GetVertState();
  if (ttype != GX_NONE)
  {
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  }

  for (i = 0; i < numMajor; ++i) 
  {
    f32 a = i * majorStep;
    f32 b = a + majorStep;
    f32 r0 = radius * (f32)sin((double)a);
    f32 r1 = radius * (f32)sin((double)b);
    f32 z0 = radius * (f32)cos((double)a);
    f32 z1 = radius * (f32)cos((double)b);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT3, (u16)((numMinor+1)*2));
    for (j = 0; j <= numMinor; ++j) 
    {
      f32 c = j * minorStep;
      f32 x = (f32)cos((double)c);
      f32 y = (f32)sin((double)c);

      GXPosition3f32(x * r1, y * r1, z1);
      GXNormal3f32((x * r1) / radius, (y * r1) / radius, z1 / radius);
      if (ttype != GX_NONE)
      {
        GXTexCoord2f32(j / (f32)numMinor, (i + 1) / (f32)numMajor);
      }

      GXPosition3f32(x * r0, y * r0, z0);
      GXNormal3f32((x * r0) / radius, (y * r0) / radius, z0 / radius);
      if (ttype != GX_NONE)
      {
        GXTexCoord2f32(j / (f32)numMinor, (i + 0) / (f32)numMajor);
      }
      
    }
    GXEnd();
  }

  RestoreVertState();
}

void GXDrawCube(void)
{
    GXAttrType  ntype, ttype;
    
    // check if binormal is active
    GXGetVtxDesc(GX_VA_NBT, &ntype);
    GXGetVtxDesc(GX_VA_TEX0, &ttype);
    GetVertState();
    
    if (ntype != GX_NONE)
    {
        GXSetVtxDesc(GX_VA_NBT, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_NBT, GX_NRM_NBT, GX_F32, 0);
    }
    if (ttype != GX_NONE)
    {
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT3, GX_VA_TEX0, GX_TEX_ST, GX_S8, 0);
    }
    
    GXBegin(GX_QUADS, GX_VTXFMT3, 4*6);
    GXDrawCubeFace(-1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, -1.0f,
                    0.0f, 1.0f, 0.0f, ntype, ttype);
                    
    GXDrawCubeFace(1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, -1.0f, ntype, ttype);
                    
    GXDrawCubeFace(0.0f, -1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, ntype, ttype);

    GXDrawCubeFace(0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 1.0f,
                    -1.0f, 0.0f, 0.0f, ntype, ttype);

    GXDrawCubeFace(0.0f, 0.0f, -1.0f,
                    0.0f, -1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f, ntype, ttype);

    GXDrawCubeFace(0.0f, 0.0f, 1.0f,
                    1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f, ntype, ttype);
    GXEnd();

    RestoreVertState();
}

void GXDrawDodeca(void)
{
    u32 i;

    GetVertState();

    for (i = 0; i < 12; ++i) 
    {
        f32 *p0, *p1, *p2;
        f32 u[3], v[3], n[3];

        p0 = &verts[polygons[i][0]][0];
        p1 = &verts[polygons[i][1]][0];
        p2 = &verts[polygons[i][2]][0];
        vsub(p1, p2, u);
        vsub(p1, p0, v);
        vcross(u, v, n);
        normalize(n);

        GXBegin(GX_TRIANGLEFAN, GX_VTXFMT3, 5);
            myvertex(verts[polygons[i][4]], n);
            myvertex(verts[polygons[i][3]], n);
            myvertex(p2, n);
            myvertex(p1, n);
            myvertex(p0, n);
        GXEnd();
      }

      RestoreVertState();
}

void GXDrawOctahedron(void)
{
    s32 i;
    
    GetVertState();

    for (i = 7; i >= 0; i--) 
        SubDivTriangle(0, (u8)i, odata, ondex);

    RestoreVertState();
}

void GXDrawIcosahedron(void)
{
    s32 i;

    GetVertState();

    for (i = 19; i >= 0; i--) 
        SubDivTriangle(0, (u8)i, idata, index);

    RestoreVertState();
}

void GXDrawSphere1(u8 depth)
{
    s32 i;

    GetVertState();

    for (i = 19; i >= 0; i--) 
        SubDivTriangle(depth, (u8)i, idata, index);

    RestoreVertState();
}

u32 GXGenNormalTable(u8 depth, f32* table)
{
    s32 i;

    nrm_cnt = 0;
    nrm_tab = table;

    // use octahedron as base shape
    for (i = 7; i >= 0; i--) 
    SubDivNrm(depth, (u8)i, odata, ondex);

    return(nrm_cnt);
}
