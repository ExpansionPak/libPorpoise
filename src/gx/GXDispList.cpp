#include "gx.hpp"

#include <dolphin/gx/GXCommandList.h>
#include <dolphin/gx/GXGeometry.h>

#define ROUNDUP32(x) (((x) + 31) & ~31)

namespace {

// Compute bytes per vertex for indexed attributes from vtxDesc
static u32 getIndexedVertexStride(GXVtxFmt /*vtxFmt*/) {
  u32 stride = 0;
  for (GXAttr attr = GX_VA_PNMTXIDX; attr < GX_VA_MAX_ATTR; attr = static_cast<GXAttr>(attr + 1)) {
    const auto type = g_gxState.vtxDesc[attr];
    if (type == GX_INDEX8) {
      stride += 1;
    } else if (type == GX_INDEX16) {
      stride += 2;
    }
  }
  return stride;
}

// Execute a single draw command from the display list. Returns bytes consumed.
static u32 executeDrawCommand(const u8* ptr, u32 remaining) {
  if (remaining < 3) return 0;
  const u8 cmd = ptr[0];
  const u32 prim = cmd & GX_OPCODE_MASK;
  const GXVtxFmt vtxFmt = static_cast<GXVtxFmt>(cmd & GX_VAT_MASK);

  // Map primitive opcode to GXPrimitive
  GXPrimitive primitive = GX_QUADS;
  switch (prim) {
    case GX_DRAW_QUADS:          primitive = GX_QUADS; break;
    case GX_DRAW_TRIANGLES:      primitive = GX_TRIANGLES; break;
    case GX_DRAW_TRIANGLE_STRIP: primitive = GX_TRIANGLESTRIP; break;
    case GX_DRAW_TRIANGLE_FAN:   primitive = GX_TRIANGLEFAN; break;
    case GX_DRAW_LINES:          primitive = GX_LINES; break;
    case GX_DRAW_LINE_STRIP:     primitive = GX_LINESTRIP; break;
    case GX_DRAW_POINTS:         primitive = GX_POINTS; break;
    default: return 0;
  }

  const u32 nVerts = (static_cast<u32>(ptr[1]) << 8) | ptr[2];
  const u32 stride = getIndexedVertexStride(vtxFmt);
  const u32 indexDataSize = nVerts * stride;
  if (remaining < 3 + indexDataSize) return 0;

  GXBegin(primitive, vtxFmt, static_cast<u16>(nVerts));

  const u8* idx = ptr + 3;
  for (u32 v = 0; v < nVerts; ++v, idx += stride) {
    u32 off = 0;
    for (GXAttr attr = GX_VA_PNMTXIDX; attr < GX_VA_MAX_ATTR; attr = static_cast<GXAttr>(attr + 1)) {
      const auto type = g_gxState.vtxDesc[attr];
      if (type == GX_INDEX8) {
        const u8 i = idx[off++];
        if (attr == GX_VA_POS) GXPosition1x8(i);
        else if (attr == GX_VA_NRM) GXNormal1x8(i);
        else if (attr == GX_VA_CLR0) GXColor1x8(i);
        else if (attr == GX_VA_CLR1) GXColor1x8(i);
        else if (attr >= GX_VA_TEX0 && attr <= GX_VA_TEX7) GXTexCoord1x8(i);
      } else if (type == GX_INDEX16) {
        const u16 i = static_cast<u16>((idx[off] << 8) | idx[off + 1]);
        off += 2;
        if (attr == GX_VA_POS) GXPosition1x16(i);
        else if (attr == GX_VA_NRM) GXNormal1x16(i);
        else if (attr == GX_VA_CLR0) GXColor1x16(i);
        else if (attr == GX_VA_CLR1) GXColor1x16(i);
        else if (attr >= GX_VA_TEX0 && attr <= GX_VA_TEX7) GXTexCoord1x16(i);
      }
    }
  }

  GXEnd();
  return 3 + indexDataSize;
}

} // namespace

extern "C" {
void GXBeginDisplayList(void* list, u32 size) {
  CHECK(!g_gxState.dynamicDlBuf, "Display list began twice!");
  g_gxState.dynamicDlBuf.emplace(static_cast<u8*>(list), size);
}

u32 GXEndDisplayList() {
  auto& dlBuf = g_gxState.dynamicDlBuf;
  size_t size = dlBuf->size();
  size_t paddedSize = ROUNDUP32(size);
  dlBuf->append_zeroes(paddedSize - size);
  dlBuf.reset();
  return static_cast<u32>(paddedSize);
}

void GXCallDisplayListLE(const void* list, u32 nbytes) {
  GXCallDisplayList(list, nbytes);
}

void GXCallDisplayList(const void* list, u32 nbytes) {
  if (!list || nbytes < 32 || (nbytes & 31) != 0) return;

  const u8* ptr = static_cast<const u8*>(list);
  const u8* end = ptr + nbytes;

  while (ptr < end) {
    u32 remaining = static_cast<u32>(end - ptr);
    const u8 cmd = ptr[0];

    if ((cmd & GX_OPCODE_MASK) >= GX_DRAW_QUADS && (cmd & GX_OPCODE_MASK) <= GX_DRAW_POINTS) {
      const u32 consumed = executeDrawCommand(ptr, remaining);
      if (consumed == 0) break;
      ptr += consumed;
    } else if (cmd == GX_NOP || (cmd & GX_OPCODE_MASK) == 0) {
      ptr += 32;
    } else {
      ptr += 32;
    }
  }
}
}
