#include "gx.hpp"

extern "C" {

// TODO GXDrawCylinder
// TODO GXDrawTorus

void GXDrawSphere(u8 numMajor, u8 numMinor) { puts("GXDrawSphere is a stub"); }

void GXDrawSphere1(u8 depth) {
  (void)depth;
  /* Stub: recursive subdivision sphere; use GXDrawSphere(8, 16) for a simple sphere. */
}

// TODO GXDrawCube
// TODO GXDrawDodeca
// TODO GXDrawOctahedron
// TODO GXDrawIcosahedron
// TODO GXGenNormalTable
}