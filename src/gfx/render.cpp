#include "render.hpp"
#include "gx_state.hpp"
#include "common.hpp"
#include "math.hpp"
#include "window.hpp"
#include <dolphin/os.h>  // For OSReport
#include <dolphin/vi.h>  // For VIMakeContextCurrent

// OpenGL includes
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#ifdef __linux__
#include <GL/glx.h>
#endif
#endif

// Define OpenGL VBO constants and functions if not available
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_DYNAMIC_DRAW 0x88E8
#endif

// Define GLsizeiptr if not available
#ifndef GLsizeiptr
typedef ptrdiff_t GLsizeiptr;
#endif

// Function pointers for VBO functions (will be loaded at runtime)
typedef void (APIENTRY *PFNGLGENBUFFERS)(GLsizei, GLuint*);
typedef void (APIENTRY *PFNGLBINDBUFFER)(GLenum, GLuint);
typedef void (APIENTRY *PFNGLBUFFERDATA)(GLenum, GLsizeiptr, const void*, GLenum);

static PFNGLGENBUFFERS glGenBuffers = nullptr;
static PFNGLBINDBUFFER glBindBuffer = nullptr;
static PFNGLBUFFERDATA glBufferData = nullptr;

// Load VBO function pointers (call once after OpenGL context is created)
static void load_vbo_functions() {
  if (glGenBuffers) return; // Already loaded
  
#ifdef _WIN32
  // wglGetProcAddress is defined in windows.h / wingdi.h
  glGenBuffers = (PFNGLGENBUFFERS)wglGetProcAddress("glGenBuffers");
  glBindBuffer = (PFNGLBINDBUFFER)wglGetProcAddress("glBindBuffer");
  glBufferData = (PFNGLBUFFERDATA)wglGetProcAddress("glBufferData");
  
  // If VBO functions aren't available, they'll be NULL and we'll use immediate mode
  if (!glGenBuffers || !glBindBuffer || !glBufferData) {
    // VBOs not available - will fall back to immediate mode
    glGenBuffers = nullptr;
    glBindBuffer = nullptr;
    glBufferData = nullptr;
  }
#elif defined(__APPLE__)
  // On macOS, these are available in OpenGL 1.5+
  glGenBuffers = ::glGenBuffers;
  glBindBuffer = ::glBindBuffer;
  glBufferData = ::glBufferData;
#else
  // Linux - use glXGetProcAddress
  extern void* glXGetProcAddress(const unsigned char*);
  glGenBuffers = (PFNGLGENBUFFERS)glXGetProcAddress((const GLubyte*)"glGenBuffers");
  glBindBuffer = (PFNGLBINDBUFFER)glXGetProcAddress((const GLubyte*)"glBindBuffer");
  glBufferData = (PFNGLBUFFERDATA)glXGetProcAddress((const GLubyte*)"glBufferData");
  
  if (!glGenBuffers || !glBindBuffer || !glBufferData) {
    glGenBuffers = nullptr;
    glBindBuffer = nullptr;
    glBufferData = nullptr;
  }
#endif
}

#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <cstdlib>  // for std::div
#include <cstddef>  // for ptrdiff_t

namespace porpoise::gfx {

using namespace porpoise::gfx::gx;

// Staging buffers for vertex/index data (similar to Aurora)
// Function-local statics avoid static init order fiasco when linked with PikminDemo
static std::vector<uint8_t>& g_vertexBuffer() {
  static std::vector<uint8_t> s;
  return s;
}
static std::vector<uint8_t>& g_indexBuffer() {
  static std::vector<uint8_t> s;
  return s;
}
static std::vector<DrawData>& g_drawCommands() {
  static std::vector<DrawData> s;
  return s;
}
static bool g_lastFrameByBridge = false;
static bool g_rendered_this_frame = false;

// OpenGL VBOs for vertex and index data
static GLuint g_vertexVBO = 0;
static GLuint g_indexVBO = 0;
static bool g_vbosInitialized = false;

// Initialize OpenGL VBOs
static void init_vbos() {
  if (g_vbosInitialized) return;
  
  load_vbo_functions();
  if (glGenBuffers && glBindBuffer && glBufferData) {
    glGenBuffers(1, &g_vertexVBO);
    glGenBuffers(1, &g_indexVBO);
    if (g_vertexVBO == 0 || g_indexVBO == 0) {
      // Failed to create VBOs - disable VBO usage
      glGenBuffers = nullptr;
      glBindBuffer = nullptr;
      glBufferData = nullptr;
    }
  }
  g_vbosInitialized = true;
}

// Convert GX primitive to OpenGL primitive
static GLenum gx_to_gl_primitive(GXPrimitive primitive) {
  switch (primitive) {
    case GX_POINTS: return GL_POINTS;
    case GX_LINES: return GL_LINES;
    case GX_LINESTRIP: return GL_LINE_STRIP;
    case GX_TRIANGLES: return GL_TRIANGLES;
    case GX_TRIANGLESTRIP: return GL_TRIANGLE_STRIP;
    case GX_TRIANGLEFAN: return GL_TRIANGLE_FAN;
    case GX_QUADS: return GL_QUADS;
    default: return GL_TRIANGLES;
  }
}

// Push vertex data to staging buffer
Range push_verts(const uint8_t* data, size_t length) {
  if (length == 0 || data == nullptr) {
    return {0, 0};
  }
  
  // Validate that we can safely read from this memory range
  // We can't easily check if the pointer is valid, but we can at least check for null
  // and reasonable size limits
  if (length > 100 * 1024 * 1024) { // 100MB limit
    return {0, 0}; // Suspiciously large, skip it
  }
  
  const uint32_t offset = static_cast<uint32_t>(g_vertexBuffer().size());
  try {
    g_vertexBuffer().insert(g_vertexBuffer().end(), data, data + length);
    // No logging - too verbose
  } catch (...) {
    // If insertion fails (e.g., out of memory), return empty range
    return {0, 0};
  }
  
  return {offset, static_cast<uint32_t>(length)};
}

// Push index data to staging buffer
Range push_indices(const uint8_t* data, size_t length) {
  if (length == 0 || data == nullptr) {
    return {0, 0};
  }
  
  // Validate that we can safely read from this memory range
  if (length > 100 * 1024 * 1024) { // 100MB limit
    return {0, 0}; // Suspiciously large, skip it
  }
  
  const uint32_t offset = static_cast<uint32_t>(g_indexBuffer().size());
  try {
    g_indexBuffer().insert(g_indexBuffer().end(), data, data + length);
  } catch (...) {
    // If insertion fails (e.g., out of memory), return empty range
    return {0, 0};
  }
  
  return {offset, static_cast<uint32_t>(length)};
}

// Queue a draw command
void push_draw_command(const DrawData& data) {
  g_drawCommands().push_back(data);
}

// Clear all queued commands (called at start of frame)
void begin_frame() {
  g_lastFrameByBridge = false;
  g_rendered_this_frame = false;
  g_drawCommands().clear();
  g_vertexBuffer().clear();
  g_indexBuffer().clear();
  // Array cached ranges point into g_vertexBuffer(), so they are invalid after clear().
  for (auto& array : g_gxState().arrays) {
    array.cachedRange = {0, 0};
  }
}

// Convert GX matrix (column-major) to OpenGL matrix (column-major, but different layout)
static void set_gl_matrix(const porpoise::Mat4x4<float>& gxMtx) {
  float glMtx[16];
  // GX uses column-major, OpenGL also uses column-major, so we can copy directly
  for (int i = 0; i < 16; ++i) {
    glMtx[i] = gxMtx.m[i];
  }
  glLoadMatrixf(glMtx);
}

// Set up OpenGL state from GX state
static void setup_gl_state() {
  // Set up depth testing
  if (g_gxState().depthCompare) {
    glEnable(GL_DEPTH_TEST);
    GLenum depthFunc = GL_LEQUAL;
    switch (g_gxState().depthFunc) {
      case GX_NEVER: depthFunc = GL_NEVER; break;
      case GX_LESS: depthFunc = GL_LESS; break;
      case GX_EQUAL: depthFunc = GL_EQUAL; break;
      case GX_LEQUAL: depthFunc = GL_LEQUAL; break;
      case GX_GREATER: depthFunc = GL_GREATER; break;
      case GX_NEQUAL: depthFunc = GL_NOTEQUAL; break;
      case GX_GEQUAL: depthFunc = GL_GEQUAL; break;
      case GX_ALWAYS: depthFunc = GL_ALWAYS; break;
    }
    glDepthFunc(depthFunc);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  
  glDepthMask(g_gxState().depthUpdate ? GL_TRUE : GL_FALSE);
  
  // Set up color/alpha writes
  glColorMask(
    g_gxState().colorUpdate ? GL_TRUE : GL_FALSE,
    g_gxState().colorUpdate ? GL_TRUE : GL_FALSE,
    g_gxState().colorUpdate ? GL_TRUE : GL_FALSE,
    g_gxState().alphaUpdate ? GL_TRUE : GL_FALSE
  );
  
  // Set up culling
  if (g_gxState().cullMode != GX_CULL_NONE) {
    glEnable(GL_CULL_FACE);
    glCullFace(g_gxState().cullMode == GX_CULL_FRONT ? GL_FRONT : GL_BACK);
  } else {
    glDisable(GL_CULL_FACE);
  }
  
  // Set up blending
  if (g_gxState().blendMode != GX_BM_NONE) {
    glEnable(GL_BLEND);
    GLenum srcFactor = GL_SRC_ALPHA;
    GLenum dstFactor = GL_ONE_MINUS_SRC_ALPHA;
    // Basic blend factor conversion (simplified)
    switch (g_gxState().blendFacSrc) {
      case GX_BL_ZERO: srcFactor = GL_ZERO; break;
      case GX_BL_ONE: srcFactor = GL_ONE; break;
      case GX_BL_SRCCLR: srcFactor = GL_SRC_COLOR; break;
      case GX_BL_INVSRCCLR: srcFactor = GL_ONE_MINUS_SRC_COLOR; break;
      case GX_BL_SRCALPHA: srcFactor = GL_SRC_ALPHA; break;
      case GX_BL_INVSRCALPHA: srcFactor = GL_ONE_MINUS_SRC_ALPHA; break;
      default: break;
    }
    switch (g_gxState().blendFacDst) {
      case GX_BL_ZERO: dstFactor = GL_ZERO; break;
      case GX_BL_ONE: dstFactor = GL_ONE; break;
      case GX_BL_SRCCLR: dstFactor = GL_SRC_COLOR; break;
      case GX_BL_INVSRCCLR: dstFactor = GL_ONE_MINUS_SRC_COLOR; break;
      case GX_BL_SRCALPHA: dstFactor = GL_SRC_ALPHA; break;
      case GX_BL_INVSRCALPHA: dstFactor = GL_ONE_MINUS_SRC_ALPHA; break;
      default: break;
    }
    glBlendFunc(srcFactor, dstFactor);
  } else {
    glDisable(GL_BLEND);
  }
}

// Execute all queued draw commands
// This is called from DEMODoneRender before GXCopyDisp
void render() {
  static int frameCount = 0;
  frameCount++;
  g_lastFrameByBridge = false;

  // When tabbed away/minimized, skip issuing GL work entirely.
  if (VIIsRenderSuspended()) {
    g_rendered_this_frame = true;
    return;
  }
  
  // Ensure OpenGL context is current before rendering
  VIMakeContextCurrent();
  
  // Set up a default viewport if not set
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  if (viewport[2] == 0 || viewport[3] == 0) {
    const auto windowSize = porpoise::window::get_window_size();
    glViewport(0, 0, static_cast<GLsizei>(windowSize.fb_width), static_cast<GLsizei>(windowSize.fb_height));
  }

  // Reset clear-affecting state before clearing. glClear obeys color/depth masks
  // and scissor, so stale GX-driven state can otherwise leave ghosted frame regions.
  glDisable(GL_SCISSOR_TEST);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  
  // Always clear so we never swap garbage (fixes flashing when draws are skipped or empty)
  glClearColor(
    g_gxState().clearColor.x(),
    g_gxState().clearColor.y(),
    g_gxState().clearColor.z(),
    g_gxState().clearColor.w()
  );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  if (g_drawCommands().empty()) {
    g_rendered_this_frame = true;
    return;
  }
  
  // Initialize VBOs if needed
  init_vbos();
  
  // Upload vertex data to VBO
  if (!g_vertexBuffer().empty() && glBindBuffer && glBufferData) {
    glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_vertexBuffer().size()), g_vertexBuffer().data(), GL_DYNAMIC_DRAW);
  }
  
  // Upload index data to VBO
  if (!g_indexBuffer().empty() && glBindBuffer && glBufferData) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indexVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_indexBuffer().size()), g_indexBuffer().data(), GL_DYNAMIC_DRAW);
  }
  
  // Set up OpenGL state
  setup_gl_state();
  // Viewport and scissor are applied per draw from cmd
  
  // Enable fixed-function pipeline
  // Temporarily disable depth test and culling to see if geometry is being drawn
  // glEnable(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  // Check OpenGL errors before we start
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    OSReport("[render] OpenGL error before setup: 0x%x\n", err);
  }
  
  // Projection and modelview are set per draw from cmd.proj and cmd.modelView
  
  // Process each draw command
  for (const auto& cmd : g_drawCommands()) {
    if (cmd.vertRange.size == 0) continue;
    
    // Validate buffer bounds
    if (cmd.vertRange.offset + cmd.vertRange.size > g_vertexBuffer().size()) {
      continue; // Invalid range, skip this command
    }
    
    // Use this draw's projection and modelview (recorded at GXEnd)
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(cmd.proj);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(cmd.modelView);
    // Apply viewport and scissor that were current when this draw was recorded
    if (cmd.viewportWidth > 0.f && cmd.viewportHeight > 0.f) {
      porpoise::gfx::set_viewport(cmd.viewportLeft, cmd.viewportTop,
        cmd.viewportWidth, cmd.viewportHeight, cmd.viewportNear, cmd.viewportFar);
    }
    if (cmd.scissorWd > 0 && cmd.scissorHt > 0) {
      porpoise::gfx::set_scissor(cmd.scissorLeft, cmd.scissorTop, cmd.scissorWd, cmd.scissorHt);
      glEnable(GL_SCISSOR_TEST);
    } else {
      glDisable(GL_SCISSOR_TEST);
    }
    // Current color for position-only draws (no color array)
    glColor4f(cmd.matColor[0], cmd.matColor[1], cmd.matColor[2], cmd.matColor[3]);
    
    // Check if we have indexed attributes that need expansion
    bool hasIndexedAttrs = false;
    for (int i = 0; i < GX_VA_MAX_ATTR; ++i) {
      if (g_gxState().vtxDesc[i] == GX_INDEX8 || g_gxState().vtxDesc[i] == GX_INDEX16) {
        if (cmd.arrayRanges[i].size > 0) {
          hasIndexedAttrs = true;
          break;
        }
      }
    }
    
    // Bind vertex buffer (only if VBOs are available)
    if (glBindBuffer && g_vertexVBO != 0) {
      glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);
    } else {
      // Ensure no VBO is bound for immediate mode
      if (glBindBuffer) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }
    
    // Set up vertex attributes based on vertex format
    // The vertex buffer contains direct attributes only (indexed attributes are in arrays)
    // We need to calculate the stride and offsets matching GXBegin's layout
    const auto& vtxFmt = g_gxState().vtxFmts[cmd.vtxFmt];
    
    // Count indexed and direct attributes (matching GXBegin logic)
    u16 numIndexedAttrs = 0;
    u16 numDirectAttrs = 0;
    for (GXAttr attr = GX_VA_POS; attr < GX_VA_MAX_ATTR; attr = static_cast<GXAttr>(attr + 1)) {
      const auto type = g_gxState().vtxDesc[attr];
      if (type == GX_DIRECT) {
        ++numDirectAttrs;
      } else if (type == GX_INDEX8 || type == GX_INDEX16) {
        ++numIndexedAttrs;
      }
    }
    
    // Calculate directStart offset (indexed attributes come first in vertex layout)
    auto [num4xAttr, rem] = std::div(numIndexedAttrs, 4);
    u32 num2xAttr = 0;
    if (rem > 2) {
      ++num4xAttr;
    } else if (rem > 0) {
      ++num2xAttr;
    }
    u32 directStart = num4xAttr * 8 + num2xAttr * 4;
    
    // Calculate vertex stride and offsets for direct attributes
    u32 vertexStride = directStart; // Start with indexed attribute space
    u32 posOffset = 0;
    u32 colorOffset = 0;
    bool hasPosition = false;
    bool hasColor = false;
    
    u32 directOffset = directStart;
    bool hasIndexedPosition = false;
    bool hasIndexedColor = false;
    GXAttrType posIndexType = GX_NONE;
    GXAttrType colorIndexType = GX_NONE;
    
    for (GXAttr attr{}; attr < GX_VA_MAX_ATTR; attr = static_cast<GXAttr>(attr + 1)) {
      const auto type = g_gxState().vtxDesc[attr];
      if (type == GX_DIRECT) {
        u32 attrSize = 0;
        
        if (attr == GX_VA_POS || attr == GX_VA_NRM) {
          attrSize = 12; // 3 floats
          if (attr == GX_VA_POS) {
            posOffset = directOffset;
            hasPosition = true;
          }
        } else if (attr == GX_VA_CLR0 || attr == GX_VA_CLR1) {
          attrSize = 16; // 4 floats (RGBA)
          if (attr == GX_VA_CLR0) {
            colorOffset = directOffset;
            hasColor = true;
          }
        } else if (attr >= GX_VA_TEX0 && attr <= GX_VA_TEX7) {
          attrSize = 8; // 2 floats
        }
        
        directOffset += attrSize;
        vertexStride += attrSize;
      } else if (type == GX_INDEX8 || type == GX_INDEX16) {
        // Track indexed attributes
        if (attr == GX_VA_POS) {
          hasIndexedPosition = true;
          posIndexType = type;
        } else if (attr == GX_VA_CLR0) {
          hasIndexedColor = true;
          colorIndexType = type;
        }
      }
    }
    
    // If we have indexed attributes and no direct attributes, expand indices into vertex data
    if (hasIndexedAttrs && !hasPosition && !hasColor) {
      // Expand indexed vertices into actual vertex data
      static int expandCount = 0;
      expandCount++;
      std::vector<float> expandedVerts;
      u32 expandedStride = 0;
      u32 expandedPosOffset = 0;
      u32 expandedColorOffset = 0;
      bool expandedHasPos = false;
      bool expandedHasColor = false;
      
      // Calculate index stride (size of index data per vertex)
      u32 indexStride = 0;
      for (GXAttr attr = GX_VA_POS; attr < GX_VA_MAX_ATTR; attr = static_cast<GXAttr>(attr + 1)) {
        const auto type = g_gxState().vtxDesc[attr];
        if (type == GX_INDEX8) {
          indexStride += 1;
        } else if (type == GX_INDEX16) {
          indexStride += 2;
        }
      }
      
      if (indexStride == 0 || cmd.vertRange.size == 0) {
        continue; // No indices to expand
      }
      
      // Calculate vertex count:
      // - If there's an index buffer (cmd.indexCount > 0), use that count
      // - Otherwise, calculate from vertRange (number of attribute index sets)
      // When using indexed attributes without an index buffer, vertRange contains
      // all the attribute indices, and we draw them sequentially
      u32 vertexCount = 0;
      if (cmd.indexCount > 0 && cmd.idxRange.size > 0) {
        // We have an index buffer - use its count
        vertexCount = cmd.indexCount;
      } else {
        // No index buffer - calculate from vertRange
        // vertRange contains attribute indices, so divide by indexStride to get vertex count
        vertexCount = cmd.vertRange.size / indexStride;
      }
      if (vertexCount == 0) {
        continue;
      }
      
      // Calculate expanded stride (position + color = 3 + 4 = 7 floats)
      u32 floatStride = 0;
      if (g_gxState().vtxDesc[GX_VA_POS] == GX_INDEX8 || g_gxState().vtxDesc[GX_VA_POS] == GX_INDEX16) {
        floatStride += 3; // 3 floats for position
        expandedHasPos = true;
        expandedPosOffset = 0;
      }
      if (g_gxState().vtxDesc[GX_VA_CLR0] == GX_INDEX8 || g_gxState().vtxDesc[GX_VA_CLR0] == GX_INDEX16) {
        floatStride += 4; // 4 floats for color
        expandedHasColor = true;
        expandedColorOffset = expandedHasPos ? 3 : 0;
      }
      
      if (floatStride == 0) {
        continue; // No attributes to expand
      }
      
      // Allocate expanded vertex buffer
      expandedVerts.resize(vertexCount * floatStride);
      
      // Get array data pointers
      const uint8_t* posArrayData = nullptr;
      u32 posArrayStride = 0;
      const uint8_t* colorArrayData = nullptr;
      u32 colorArrayStride = 0;
      
      if (expandedHasPos && cmd.arrayRanges[GX_VA_POS].size > 0) {
        posArrayData = g_vertexBuffer().data() + cmd.arrayRanges[GX_VA_POS].offset;
        posArrayStride = g_gxState().arrays[GX_VA_POS].stride;
      }
      if (expandedHasColor && cmd.arrayRanges[GX_VA_CLR0].size > 0) {
        colorArrayData = g_vertexBuffer().data() + cmd.arrayRanges[GX_VA_CLR0].offset;
        colorArrayStride = g_gxState().arrays[GX_VA_CLR0].stride;
      }
      
      // Get index data from vertex buffer
      const uint8_t* indexData = g_vertexBuffer().data() + cmd.vertRange.offset;
      
      // Calculate position index offset in vertex layout
      u32 posIndexOffset = 0;
      u32 colorIndexOffset = 0;
      u32 currentOffset = 0;
      for (GXAttr attr = GX_VA_POS; attr < GX_VA_MAX_ATTR; attr = static_cast<GXAttr>(attr + 1)) {
        const auto type = g_gxState().vtxDesc[attr];
        if (type == GX_INDEX8) {
          if (attr == GX_VA_POS) posIndexOffset = currentOffset;
          if (attr == GX_VA_CLR0) colorIndexOffset = currentOffset;
          currentOffset += 1;
        } else if (type == GX_INDEX16) {
          if (attr == GX_VA_POS) posIndexOffset = currentOffset;
          if (attr == GX_VA_CLR0) colorIndexOffset = currentOffset;
          currentOffset += 2;
        }
      }
      
      // When we have indexed attributes:
      // - cmd.vertRange contains the attribute indices (one set per vertex)
      //   Each set contains: [posIndex, colorIndex, ...] based on indexStride
      // - cmd.idxRange (if present) is a separate index buffer that selects which vertices to draw
      // - For each vertex to draw:
      //   1. Get the attribute index set (either from idxRange or sequentially)
      //   2. Read the position/color indices from that set
      //   3. Look up actual data in the arrays using: base_ptr + attr_idx * stride
      
      const u16* vertexIndexBuffer = nullptr;
      bool useVertexIndexBuffer = (cmd.indexCount > 0 && cmd.idxRange.size > 0);
      if (useVertexIndexBuffer) {
        vertexIndexBuffer = reinterpret_cast<const u16*>(g_indexBuffer().data() + cmd.idxRange.offset);
      }
      
      // Calculate how many attribute index sets we have in vertRange
      u32 numAttributeIndexSets = cmd.vertRange.size / indexStride;
      
      // Expand vertices: for each vertex to draw, look up its attribute indices and expand
      for (u32 v = 0; v < vertexCount; ++v) {
        float* vertOut = expandedVerts.data() + (v * floatStride);
        
        // Determine which attribute index set to use for this vertex
        u32 attrIndexSet = v;
        if (useVertexIndexBuffer && vertexIndexBuffer != nullptr) {
          // Use the vertex index buffer to select which attribute index set
          attrIndexSet = vertexIndexBuffer[v];
          
          // Bounds check
          if (attrIndexSet >= numAttributeIndexSets) {
            // Invalid index - zero out this vertex and continue
            if (expandedHasPos) {
              vertOut[0] = 0.0f;
              vertOut[1] = 0.0f;
              vertOut[2] = 0.0f;
            }
            if (expandedHasColor) {
              u32 colorOffset = expandedHasPos ? 3 : 0;
              vertOut[colorOffset + 0] = 0.0f;
              vertOut[colorOffset + 1] = 0.0f;
              vertOut[colorOffset + 2] = 0.0f;
              vertOut[colorOffset + 3] = 1.0f;
            }
            continue;
          }
        } else {
          // No vertex index buffer - use sequential indices
          if (attrIndexSet >= numAttributeIndexSets) {
            // Out of bounds - zero out and continue
            if (expandedHasPos) {
              vertOut[0] = 0.0f;
              vertOut[1] = 0.0f;
              vertOut[2] = 0.0f;
            }
            if (expandedHasColor) {
              u32 colorOffset = expandedHasPos ? 3 : 0;
              vertOut[colorOffset + 0] = 0.0f;
              vertOut[colorOffset + 1] = 0.0f;
              vertOut[colorOffset + 2] = 0.0f;
              vertOut[colorOffset + 3] = 1.0f;
            }
            continue;
          }
        }
        
        // Calculate offset into the attribute index data for this vertex
        u32 indexBase = attrIndexSet * indexStride;
        
        // Bounds check
        if (indexBase + indexStride > cmd.vertRange.size) {
          continue; // Skip invalid indices
        }
        
        // Read position index (8-bit) and look up actual position data
        if (expandedHasPos && posArrayData && posIndexOffset < indexStride) {
          u8 posIndex = indexData[indexBase + posIndexOffset];
          // Calculate address: base_ptr + attr_idx * stride (from GXSetArray docs)
          u32 posDataOffset = posIndex * posArrayStride;
          // Check bounds: need at least 6 bytes (3 s16 values)
          if (posDataOffset + 6 <= cmd.arrayRanges[GX_VA_POS].size) {
            const s16* posSrc = reinterpret_cast<const s16*>(posArrayData + posDataOffset);
            vertOut[0] = static_cast<float>(posSrc[0]);
            vertOut[1] = static_cast<float>(posSrc[1]);
            vertOut[2] = static_cast<float>(posSrc[2]);
          } else {
            // Out of bounds - zero out position
            vertOut[0] = 0.0f;
            vertOut[1] = 0.0f;
            vertOut[2] = 0.0f;
          }
        }
        
        // Read color index (8-bit) and look up actual color data
        if (expandedHasColor && colorArrayData && colorIndexOffset < indexStride) {
          u8 colorIndex = indexData[indexBase + colorIndexOffset];
          // Calculate address: base_ptr + attr_idx * stride
          u32 colorDataOffset = colorIndex * colorArrayStride;
          // Check bounds: need at least 4 bytes (RGBA8)
          if (colorDataOffset + 4 <= cmd.arrayRanges[GX_VA_CLR0].size) {
            const u8* colorSrc = reinterpret_cast<const u8*>(colorArrayData + colorDataOffset);
            u32 colorOffset = expandedHasPos ? 3 : 0;
            vertOut[colorOffset + 0] = static_cast<float>(colorSrc[0]) / 255.0f;
            vertOut[colorOffset + 1] = static_cast<float>(colorSrc[1]) / 255.0f;
            vertOut[colorOffset + 2] = static_cast<float>(colorSrc[2]) / 255.0f;
            vertOut[colorOffset + 3] = static_cast<float>(colorSrc[3]) / 255.0f;
          } else {
            // Out of bounds - use white
            u32 colorOffset = expandedHasPos ? 3 : 0;
            vertOut[colorOffset + 0] = 1.0f;
            vertOut[colorOffset + 1] = 1.0f;
            vertOut[colorOffset + 2] = 1.0f;
            vertOut[colorOffset + 3] = 1.0f;
          }
        }
      }
      
      
      expandedStride = floatStride * sizeof(float);
      hasPosition = expandedHasPos;
      hasColor = expandedHasColor;
      posOffset = expandedPosOffset * sizeof(float);
      colorOffset = expandedColorOffset * sizeof(float);
      vertexStride = expandedStride;
      
      // Use expanded vertex data for rendering
      if (glBindBuffer && g_vertexVBO != 0) {
        // Upload expanded data to VBO
        glBindBuffer(GL_ARRAY_BUFFER, g_vertexVBO);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(expandedVerts.size() * sizeof(float)), 
                     expandedVerts.data(), GL_DYNAMIC_DRAW);
        
        if (hasPosition) {
          glEnableClientState(GL_VERTEX_ARRAY);
          glVertexPointer(3, GL_FLOAT, vertexStride, reinterpret_cast<const void*>(static_cast<uintptr_t>(posOffset)));
        } else {
          glDisableClientState(GL_VERTEX_ARRAY);
        }
        
        if (hasColor) {
          glEnableClientState(GL_COLOR_ARRAY);
          glColorPointer(4, GL_FLOAT, vertexStride, reinterpret_cast<const void*>(static_cast<uintptr_t>(colorOffset)));
        } else {
          glDisableClientState(GL_COLOR_ARRAY);
        }
      } else {
        // Immediate mode - use direct pointer
        if (hasPosition) {
          glEnableClientState(GL_VERTEX_ARRAY);
          glVertexPointer(3, GL_FLOAT, vertexStride, expandedVerts.data() + (posOffset / sizeof(float)));
        } else {
          glDisableClientState(GL_VERTEX_ARRAY);
        }
        
        if (hasColor) {
          glEnableClientState(GL_COLOR_ARRAY);
          glColorPointer(4, GL_FLOAT, vertexStride, expandedVerts.data() + (colorOffset / sizeof(float)));
        } else {
          glDisableClientState(GL_COLOR_ARRAY);
        }
      }
      
      // Draw using expanded vertices (non-indexed since we've expanded)
      GLenum glPrim = gx_to_gl_primitive(cmd.primitive);
      glDrawArrays(glPrim, 0, vertexCount);
      
      GLenum err = glGetError();
      if (err != GL_NO_ERROR && frameCount <= 3) {
        OSReport("[render] OpenGL error after glDrawArrays: 0x%x\n", err);
      }
      
      continue; // Skip the normal rendering path below
    }
    
    // Direct draws (indexedStride == 0): use per-draw layout from command, not current vtxDesc.
    // vtxDesc is global and gets overwritten by later draws in the same frame.
    if (cmd.indexedStride == 0) {
      const u32 posStride = cmd.posArrayStride > 0 ? cmd.posArrayStride : 12u;
      const u32 clrStride = cmd.colorArrayStride;
      vertexStride = posStride + clrStride;
      hasPosition = (posStride >= 12);
      hasColor = (clrStride >= 4);
      posOffset = 0;
      colorOffset = posStride;
    }
    
    // Set up vertex pointers
    if (glBindBuffer && g_vertexVBO != 0) {
      // Using VBOs - pointer is an offset from the start of the VBO
      if (hasPosition) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, vertexStride, 
                        reinterpret_cast<const void*>(static_cast<uintptr_t>(cmd.vertRange.offset + posOffset)));
      } else {
        glDisableClientState(GL_VERTEX_ARRAY);
      }
      
      if (hasColor) {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, vertexStride,
                       reinterpret_cast<const void*>(static_cast<uintptr_t>(cmd.vertRange.offset + colorOffset)));
      } else {
        glDisableClientState(GL_COLOR_ARRAY);
      }
    } else {
      // Immediate mode - pointer is absolute address
      const uint8_t* basePtr = g_vertexBuffer().data() + cmd.vertRange.offset;
      if (hasPosition) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, vertexStride, basePtr + posOffset);
      } else {
        glDisableClientState(GL_VERTEX_ARRAY);
      }
      
      if (hasColor) {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, vertexStride, basePtr + colorOffset);
      } else {
        glDisableClientState(GL_COLOR_ARRAY);
      }
    }
    
    // Convert GX primitive to OpenGL
    GLenum glPrim = gx_to_gl_primitive(cmd.primitive);
    
    // Draw
    if (cmd.indexCount > 0 && cmd.idxRange.size > 0) {
      // Validate index buffer bounds
      if (cmd.idxRange.offset + cmd.idxRange.size > g_indexBuffer().size()) {
        continue; // Invalid range, skip this command
      }
      
      // Indexed drawing
      if (glBindBuffer && g_indexVBO != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indexVBO);
        glDrawElements(glPrim, cmd.indexCount, GL_UNSIGNED_SHORT, 
                      reinterpret_cast<const void*>(static_cast<uintptr_t>(cmd.idxRange.offset)));
      } else {
        // Immediate mode - use direct pointer
        const u16* indices = reinterpret_cast<const u16*>(g_indexBuffer().data() + cmd.idxRange.offset);
        glDrawElements(glPrim, cmd.indexCount, GL_UNSIGNED_SHORT, indices);
      }
    } else {
      // Non-indexed drawing
      // Calculate vertex count from vertex buffer size and stride
      if (vertexStride > 0) {
        u32 vertexCount = cmd.vertRange.size / vertexStride;
        if (vertexCount > 0) {
          glDrawArrays(glPrim, 0, vertexCount);
        }
      }
    }
  }
  
  // Clean up
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  if (glBindBuffer) {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  g_rendered_this_frame = true;
}

void flush_render_if_pending() {
  if (!g_rendered_this_frame && !g_drawCommands().empty()) {
    render();
  }
}

void render_before_copy() {
  if (!g_drawCommands().empty()) {
    render();
  }
}

bool did_render_with_bridge() {
  return g_lastFrameByBridge;
}

} // namespace porpoise::gfx
