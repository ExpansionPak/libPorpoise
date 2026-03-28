#include "gx.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern "C" {

static void gx_emit_triangle_with_face_normal(const f32 a[3], const f32 b[3], const f32 c[3]) {
  const f32 ux = b[0] - a[0];
  const f32 uy = b[1] - a[1];
  const f32 uz = b[2] - a[2];
  const f32 vx = c[0] - a[0];
  const f32 vy = c[1] - a[1];
  const f32 vz = c[2] - a[2];
  f32 nx = uy * vz - uz * vy;
  f32 ny = uz * vx - ux * vz;
  f32 nz = ux * vy - uy * vx;
  const f32 len2 = nx * nx + ny * ny + nz * nz;
  if (len2 > 1e-12f) {
    const f32 inv_len = 1.0f / std::sqrt(len2);
    nx *= inv_len;
    ny *= inv_len;
    nz *= inv_len;
  } else {
    nx = 0.0f;
    ny = 0.0f;
    nz = 1.0f;
  }

  GXPosition3f32(a[0], a[1], a[2]);
  GXNormal3f32(nx, ny, nz);
  GXPosition3f32(b[0], b[1], b[2]);
  GXNormal3f32(nx, ny, nz);
  GXPosition3f32(c[0], c[1], c[2]);
  GXNormal3f32(nx, ny, nz);
}

void GXDrawCylinder(u8 numEdges) {
  if (numEdges < 3) numEdges = 3;
  const f32 two_pi = 2.0f * static_cast<f32>(M_PI);

  GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<u16>((numEdges + 1) * 2));
  for (u32 i = 0; i <= numEdges; ++i) {
    const f32 t = two_pi * static_cast<f32>(i) / static_cast<f32>(numEdges);
    const f32 x = std::cos(t);
    const f32 y = std::sin(t);
    GXPosition3f32(x, y, 1.0f);
    GXNormal3f32(x, y, 0.0f);
    GXPosition3f32(x, y, -1.0f);
    GXNormal3f32(x, y, 0.0f);
  }
  GXEnd();

  GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast<u16>(numEdges + 2));
  GXPosition3f32(0.0f, 0.0f, 1.0f);
  GXNormal3f32(0.0f, 0.0f, 1.0f);
  for (u32 i = 0; i <= numEdges; ++i) {
    const f32 t = two_pi * static_cast<f32>(i) / static_cast<f32>(numEdges);
    GXPosition3f32(std::cos(t), std::sin(t), 1.0f);
    GXNormal3f32(0.0f, 0.0f, 1.0f);
  }
  GXEnd();

  GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, static_cast<u16>(numEdges + 2));
  GXPosition3f32(0.0f, 0.0f, -1.0f);
  GXNormal3f32(0.0f, 0.0f, -1.0f);
  for (s32 i = numEdges; i >= 0; --i) {
    const f32 t = two_pi * static_cast<f32>(i) / static_cast<f32>(numEdges);
    GXPosition3f32(std::cos(t), std::sin(t), -1.0f);
    GXNormal3f32(0.0f, 0.0f, -1.0f);
  }
  GXEnd();
}

void GXDrawTorus(f32 rc, u8 numc, u8 numt) {
  if (numc < 3) numc = 3;
  if (numt < 3) numt = 3;
  if (rc <= 0.0f) rc = 0.25f;

  const f32 two_pi = 2.0f * static_cast<f32>(M_PI);

  for (u32 i = 0; i < numc; ++i) {
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<u16>((numt + 1) * 2));
    for (u32 j = 0; j <= numt; ++j) {
      for (u32 k = 1; k != static_cast<u32>(-1); --k) {
        const f32 s = static_cast<f32>((i + k) % numc);
        const f32 t = static_cast<f32>(j % numt);
        const f32 theta = two_pi * s / static_cast<f32>(numc);
        const f32 phi = two_pi * t / static_cast<f32>(numt);
        const f32 ct = std::cos(theta);
        const f32 st = std::sin(theta);
        const f32 cp = std::cos(phi);
        const f32 sp = std::sin(phi);
        const f32 r = 1.0f + rc * ct;

        GXPosition3f32(r * cp, r * sp, rc * st);
        GXNormal3f32(ct * cp, ct * sp, st);
      }
    }
    GXEnd();
  }
}

void GXDrawSphere(u8 numMajor, u8 numMinor) {
  if (numMajor < 2) numMajor = 2;
  if (numMinor < 3) numMinor = 3;
  const f32 radius = 1.0f;
  const f32 majorStep = static_cast<f32>(M_PI) / numMajor;
  const f32 minorStep = 2.0f * static_cast<f32>(M_PI) / numMinor;

  for (s32 i = 0; i < numMajor; ++i) {
    const f32 a = i * majorStep;
    const f32 b = a + majorStep;
    const f32 r0 = radius * std::sin(a);
    const f32 r1 = radius * std::sin(b);
    const f32 z0 = radius * std::cos(a);
    const f32 z1 = radius * std::cos(b);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<u16>((numMinor + 1) * 2));
    for (s32 j = 0; j <= numMinor; ++j) {
      const f32 c = j * minorStep;
      const f32 x = std::cos(c);
      const f32 y = std::sin(c);

      GXPosition3f32(x * r1, y * r1, z1);
      GXNormal3f32(x * r1 / radius, y * r1 / radius, z1 / radius);

      GXPosition3f32(x * r0, y * r0, z0);
      GXNormal3f32(x * r0 / radius, y * r0 / radius, z0 / radius);
    }
    GXEnd();
  }
}

void GXDrawSphere1(u8 depth) {
  u8 nMajor = 8;
  u8 nMinor = 16;
  if (depth >= 3) {
    nMajor = 16;
    nMinor = 24;
  } else if (depth >= 2) {
    nMajor = 12;
    nMinor = 20;
  } else if (depth >= 1) {
    nMajor = 8;
    nMinor = 16;
  } else {
    nMajor = 6;
    nMinor = 12;
  }
  GXDrawSphere(nMajor, nMinor);
}

void GXDrawCube(void) {
  static const f32 verts[8][3] = {
      {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
      {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f},
  };
  static const u8 faces[6][4] = {
      {0, 1, 2, 3}, {5, 4, 7, 6}, {4, 0, 3, 7}, {1, 5, 6, 2}, {4, 5, 1, 0}, {3, 2, 6, 7},
  };
  static const f32 norms[6][3] = {
      {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f},
      {1.0f, 0.0f, 0.0f},  {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
  };

  GXBegin(GX_QUADS, GX_VTXFMT0, 24);
  for (u32 f = 0; f < 6; ++f) {
    for (u32 i = 0; i < 4; ++i) {
      const u8 vi = faces[f][i];
      GXPosition3f32(verts[vi][0], verts[vi][1], verts[vi][2]);
      GXNormal3f32(norms[f][0], norms[f][1], norms[f][2]);
    }
  }
  GXEnd();
}

void GXDrawOctahedron(void) {
  static const f32 v[6][3] = {
      {1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
      {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f},
  };
  static const u8 tri[8][3] = {
      {0, 2, 4}, {2, 1, 4}, {1, 3, 4}, {3, 0, 4}, {2, 0, 5}, {1, 2, 5}, {3, 1, 5}, {0, 3, 5},
  };

  GXBegin(GX_TRIANGLES, GX_VTXFMT0, 24);
  for (u32 i = 0; i < 8; ++i) {
    gx_emit_triangle_with_face_normal(v[tri[i][0]], v[tri[i][1]], v[tri[i][2]]);
  }
  GXEnd();
}

void GXDrawIcosahedron(void) {
  static const f32 phi = 1.6180339887498948f;
  static const f32 raw[12][3] = {
      {-1.0f, phi, 0.0f},  {1.0f, phi, 0.0f},   {-1.0f, -phi, 0.0f}, {1.0f, -phi, 0.0f},
      {0.0f, -1.0f, phi},  {0.0f, 1.0f, phi},   {0.0f, -1.0f, -phi}, {0.0f, 1.0f, -phi},
      {phi, 0.0f, -1.0f},  {phi, 0.0f, 1.0f},   {-phi, 0.0f, -1.0f}, {-phi, 0.0f, 1.0f},
  };
  static const u8 tri[20][3] = {
      {0, 11, 5}, {0, 5, 1},  {0, 1, 7},  {0, 7, 10}, {0, 10, 11},
      {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
      {3, 9, 4},  {3, 4, 2},  {3, 2, 6},  {3, 6, 8},  {3, 8, 9},
      {4, 9, 5},  {2, 4, 11}, {6, 2, 10}, {8, 6, 7},  {9, 8, 1},
  };
  f32 v[12][3];
  for (u32 i = 0; i < 12; ++i) {
    const f32 len = std::sqrt(raw[i][0] * raw[i][0] + raw[i][1] * raw[i][1] + raw[i][2] * raw[i][2]);
    const f32 inv = (len > 1e-12f) ? (1.0f / len) : 1.0f;
    v[i][0] = raw[i][0] * inv;
    v[i][1] = raw[i][1] * inv;
    v[i][2] = raw[i][2] * inv;
  }

  GXBegin(GX_TRIANGLES, GX_VTXFMT0, 60);
  for (u32 i = 0; i < 20; ++i) {
    gx_emit_triangle_with_face_normal(v[tri[i][0]], v[tri[i][1]], v[tri[i][2]]);
  }
  GXEnd();
}

void GXDrawDodeca(void) {
  static const f32 raw[12][3] = {
      {-1.0f, 1.6180339887498948f, 0.0f},  {1.0f, 1.6180339887498948f, 0.0f},
      {-1.0f, -1.6180339887498948f, 0.0f}, {1.0f, -1.6180339887498948f, 0.0f},
      {0.0f, -1.0f, 1.6180339887498948f},  {0.0f, 1.0f, 1.6180339887498948f},
      {0.0f, -1.0f, -1.6180339887498948f}, {0.0f, 1.0f, -1.6180339887498948f},
      {1.6180339887498948f, 0.0f, -1.0f},  {1.6180339887498948f, 0.0f, 1.0f},
      {-1.6180339887498948f, 0.0f, -1.0f}, {-1.6180339887498948f, 0.0f, 1.0f},
  };
  static const u8 tri[20][3] = {
      {0, 11, 5}, {0, 5, 1},  {0, 1, 7},  {0, 7, 10}, {0, 10, 11},
      {1, 5, 9},  {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
      {3, 9, 4},  {3, 4, 2},  {3, 2, 6},  {3, 6, 8},  {3, 8, 9},
      {4, 9, 5},  {2, 4, 11}, {6, 2, 10}, {8, 6, 7},  {9, 8, 1},
  };
  f32 ico_v[12][3];
  f32 dod_v[20][3];
  u8 incident[12][5] = {};
  u8 incident_count[12] = {};

  for (u32 i = 0; i < 12; ++i) {
    const f32 x = raw[i][0];
    const f32 y = raw[i][1];
    const f32 z = raw[i][2];
    const f32 inv = 1.0f / std::sqrt(x * x + y * y + z * z);
    ico_v[i][0] = x * inv;
    ico_v[i][1] = y * inv;
    ico_v[i][2] = z * inv;
  }

  for (u32 f = 0; f < 20; ++f) {
    const u8 a = tri[f][0];
    const u8 b = tri[f][1];
    const u8 c = tri[f][2];
    const f32 cx = ico_v[a][0] + ico_v[b][0] + ico_v[c][0];
    const f32 cy = ico_v[a][1] + ico_v[b][1] + ico_v[c][1];
    const f32 cz = ico_v[a][2] + ico_v[b][2] + ico_v[c][2];
    const f32 inv = 1.0f / std::sqrt(cx * cx + cy * cy + cz * cz);
    dod_v[f][0] = cx * inv;
    dod_v[f][1] = cy * inv;
    dod_v[f][2] = cz * inv;
    if (incident_count[a] < 5) incident[a][incident_count[a]++] = static_cast<u8>(f);
    if (incident_count[b] < 5) incident[b][incident_count[b]++] = static_cast<u8>(f);
    if (incident_count[c] < 5) incident[c][incident_count[c]++] = static_cast<u8>(f);
  }

  GXBegin(GX_TRIANGLES, GX_VTXFMT0, 108);
  for (u32 i = 0; i < 12; ++i) {
    if (incident_count[i] != 5) continue;

    int order[5] = {0, 1, 2, 3, 4};
    f32 angle[5];
    u8 face_ids[5];
    const f32 nx = ico_v[i][0];
    const f32 ny = ico_v[i][1];
    const f32 nz = ico_v[i][2];
    f32 tx, ty, tz;
    if (std::fabs(nz) < 0.9f) {
      tx = -ny;
      ty = nx;
      tz = 0.0f;
    } else {
      tx = 0.0f;
      ty = -nz;
      tz = ny;
    }
    {
      const f32 inv_t = 1.0f / std::sqrt(tx * tx + ty * ty + tz * tz);
      tx *= inv_t;
      ty *= inv_t;
      tz *= inv_t;
    }
    const f32 bx = ny * tz - nz * ty;
    const f32 by = nz * tx - nx * tz;
    const f32 bz = nx * ty - ny * tx;

    for (int j = 0; j < 5; ++j) {
      const u8 vid = incident[i][j];
      const f32 dx = dod_v[vid][0];
      const f32 dy = dod_v[vid][1];
      const f32 dz = dod_v[vid][2];
      const f32 dproj = dx * nx + dy * ny + dz * nz;
      const f32 px = dx - dproj * nx;
      const f32 py = dy - dproj * ny;
      const f32 pz = dz - dproj * nz;
      const f32 x = px * tx + py * ty + pz * tz;
      const f32 y = px * bx + py * by + pz * bz;
      angle[j] = std::atan2(y, x);
    }

    for (int a = 0; a < 4; ++a) {
      for (int b = a + 1; b < 5; ++b) {
        if (angle[order[a]] > angle[order[b]]) {
          const int t = order[a];
          order[a] = order[b];
          order[b] = t;
        }
      }
    }
    for (int j = 0; j < 5; ++j) face_ids[j] = incident[i][order[j]];

    {
      const f32* p0 = dod_v[face_ids[0]];
      const f32* p1 = dod_v[face_ids[1]];
      const f32* p2 = dod_v[face_ids[2]];
      const f32 ux = p1[0] - p0[0];
      const f32 uy = p1[1] - p0[1];
      const f32 uz = p1[2] - p0[2];
      const f32 vx = p2[0] - p0[0];
      const f32 vy = p2[1] - p0[1];
      const f32 vz = p2[2] - p0[2];
      const f32 fx = uy * vz - uz * vy;
      const f32 fy = uz * vx - ux * vz;
      const f32 fz = ux * vy - uy * vx;
      if (fx * nx + fy * ny + fz * nz < 0.0f) {
        const u8 t1 = face_ids[1];
        face_ids[1] = face_ids[4];
        face_ids[4] = t1;
        const u8 t2 = face_ids[2];
        face_ids[2] = face_ids[3];
        face_ids[3] = t2;
      }
    }

    gx_emit_triangle_with_face_normal(dod_v[face_ids[0]], dod_v[face_ids[1]], dod_v[face_ids[2]]);
    gx_emit_triangle_with_face_normal(dod_v[face_ids[0]], dod_v[face_ids[2]], dod_v[face_ids[3]]);
    gx_emit_triangle_with_face_normal(dod_v[face_ids[0]], dod_v[face_ids[3]], dod_v[face_ids[4]]);
  }
  GXEnd();
}

// TODO GXGenNormalTable
}
