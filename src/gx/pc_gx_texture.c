/* pc_gx_texture.c - GC texture format decoders + 2048-entry texture cache */
#include "pc_gx_internal.h"
#include "pc_texture_pack.h"
#include <dolphin/gx/GXTexture.h>
#include <dolphin/gx/GXGet.h>
#include <stdlib.h>

#define PC_GX_TLUT_SLOTS 20

static int pc_gx_tlut_force_be(void);
static void decode_rgb5a3_entry(u16 val, u8* r, u8* g, u8* b, u8* a);
static u32 tlut_content_hash(const void* data, int tlut_fmt, int n_entries, int is_be);
static GXTexRegion* default_tex_region_callback(GXTexObj* obj, GXTexMapID id);
static GXTlutRegion* default_tlut_region_callback(u32 tlut_name);

/* --- TLUT stale-data detection ---
 * On GC, gsDPLoadTLUT_Dolphin always re-DMA'd palette data from memory.
 * emu64 optimizes by skipping reload when the TLUT address hasn't changed.
 * On PC, the game reuses memory buffers — same address can hold different
 * palette data (e.g., different NPC clothing). We track the first u16 of
 * each TLUT slot to detect content changes and force reloads. */
u16 s_tlut_first_word[16];

/* --- texture cache --- */
#define TEX_CACHE_SIZE 2048

typedef struct {
    uintptr_t data_ptr;
    u16 width;
    u16 height;
    u32 format;
    u32 tlut_name;   /* TLUT slot for CI4/CI8/CI14X2, 0xFFFFFFFF for non-indexed */
    uintptr_t tlut_ptr;
    u32 tlut_hash;   /* FNV-1a of TLUT data + metadata */
    u32 data_hash;   /* FNV-1a of first N bytes */
    GLuint gl_tex;
    u32 wrap_s;
    u32 wrap_t;
    u32 min_filter;
    u8 external;     /* owned by texture pack, don't delete on eviction */
} TexCacheEntry;

/* Bits-per-pixel for each GC texture format (8 for unknown as safe default) */
static int gc_format_bpp(u32 format) {
    switch (format) {
        case GX_TF_I4:
        case GX_TF_C4:
        case GX_TF_CMPR:
        case GX_CTF_R4:
        case GX_CTF_Z4:
            return 4;

        case GX_TF_I8:
        case GX_TF_IA4:
        case GX_TF_C8:
        case GX_TF_Z8:
        case GX_CTF_RA4:
        case GX_CTF_A8:
        case GX_CTF_R8:
        case GX_CTF_G8:
        case GX_CTF_B8:
        case GX_CTF_Z8M:
        case GX_CTF_Z8L:
            return 8;

        case GX_TF_IA8:
        case GX_TF_RGB565:
        case GX_TF_RGB5A3:
        case GX_TF_C14X2:
        case GX_TF_Z16:
        case GX_CTF_RA8:
        case GX_CTF_RG8:
        case GX_CTF_GB8:
        case GX_CTF_Z16L:
            return 16;

        case GX_TF_RGBA8:
        case GX_TF_Z24X8:
        case GX_CTF_YUVA8:
            return 32;

        default: return 8;
    }
}

/* FNV-1a hash of texture data to detect buffer reuse with different content.
 * Hashes first 256 + last 256 bytes (or all if <= 512). */
static u32 tex_content_hash(const void* data, int width, int height, u32 format) {
    if (!data) return 0;
    int bpp = gc_format_bpp(format);
    int data_size = (width * height * bpp) / 8;
    const u8* p = (const u8*)data;
    u32 h = 0x811c9dc5u;
    if (data_size <= 512) {
        /* small texture: hash everything */
        for (int i = 0; i < data_size; i++) {
            h ^= p[i];
            h *= 0x01000193u;
        }
    } else {
        /* large texture: sample head + tail */
        for (int i = 0; i < 256; i++) {
            h ^= p[i];
            h *= 0x01000193u;
        }
        for (int i = data_size - 256; i < data_size; i++) {
            h ^= p[i];
            h *= 0x01000193u;
        }
    }
    return h;
}

/* FNV-1a hash of TLUT data + metadata. CI textures need palette identity
 * in the cache key since the same image is often reused with different TLUTs. */
static u32 tlut_content_hash(const void* data, int tlut_fmt, int n_entries, int is_be) {
    if (!data || n_entries <= 0) return 0;
    int bytes = n_entries * 2;
    if (bytes > 512) bytes = 512;
    const u8* p = (const u8*)data;
    u32 h = 0x811c9dc5u;
    for (int i = 0; i < bytes; i++) {
        h ^= p[i];
        h *= 0x01000193u;
    }
    /* mix in metadata to distinguish different TLUT modes */
    h ^= (u32)(tlut_fmt & 0xFF); h *= 0x01000193u;
    h ^= (u32)(n_entries & 0xFFFF); h *= 0x01000193u;
    h ^= (u32)(is_be & 1); h *= 0x01000193u;
    return h;
}

static TexCacheEntry tex_cache[TEX_CACHE_SIZE];
static int tex_cache_count = 0;
static int tex_cache_hits = 0;
static int tex_cache_misses = 0;

#define TEXOBJ_USERDATA_SLOTS 1024
typedef struct {
    const void* obj_ptr;
    const void* user_data;
} TexObjUserDataEntry;
static TexObjUserDataEntry s_texobj_user_data[TEXOBJ_USERDATA_SLOTS];
static GXTexRegion s_default_tex_regions[GX_MAX_TEXMAP];
static GXTlutRegion s_default_tlut_regions[GX_BIGTLUT3 + 1];
static GXTexRegionCallback s_tex_region_callback = default_tex_region_callback;
static GXTlutRegionCallback s_tlut_region_callback = default_tlut_region_callback;
static int s_skip_region_callbacks = 0;

static GXTexRegion* default_tex_region_callback(GXTexObj* obj, GXTexMapID id) {
    (void)obj;
    return &s_default_tex_regions[(u32)id % GX_MAX_TEXMAP];
}

static GXTlutRegion* default_tlut_region_callback(u32 tlut_name) {
    return &s_default_tlut_regions[tlut_name % (GX_BIGTLUT3 + 1)];
}

static void texobj_user_data_set(const void* obj_ptr, const void* user_data) {
    int first_free = -1;
    for (int i = 0; i < TEXOBJ_USERDATA_SLOTS; ++i) {
        if (s_texobj_user_data[i].obj_ptr == obj_ptr) {
            s_texobj_user_data[i].user_data = user_data;
            return;
        }
        if (first_free < 0 && s_texobj_user_data[i].obj_ptr == NULL) {
            first_free = i;
        }
    }

    if (first_free >= 0) {
        s_texobj_user_data[first_free].obj_ptr = obj_ptr;
        s_texobj_user_data[first_free].user_data = user_data;
        return;
    }

    /* fallback replacement when table is full */
    s_texobj_user_data[((uintptr_t)obj_ptr >> 2) % TEXOBJ_USERDATA_SLOTS].obj_ptr = obj_ptr;
    s_texobj_user_data[((uintptr_t)obj_ptr >> 2) % TEXOBJ_USERDATA_SLOTS].user_data = user_data;
}

static const void* texobj_user_data_get(const void* obj_ptr) {
    for (int i = 0; i < TEXOBJ_USERDATA_SLOTS; ++i) {
        if (s_texobj_user_data[i].obj_ptr == obj_ptr) {
            return s_texobj_user_data[i].user_data;
        }
    }
    return NULL;
}

/* Linear scan. Fine for <=2048 entries at ~100% hit rate. */
static TexCacheEntry* tex_cache_find(uintptr_t data_ptr, int w, int h, u32 fmt, u32 tlut_name,
                                     uintptr_t tlut_ptr, u32 tlut_hash, u32 data_hash) {
    for (int i = 0; i < tex_cache_count; i++) {
        TexCacheEntry* e = &tex_cache[i];
        if (e->data_ptr == data_ptr && e->width == w && e->height == h &&
            e->format == fmt && e->tlut_name == tlut_name && e->tlut_ptr == tlut_ptr &&
            e->tlut_hash == tlut_hash && e->data_hash == data_hash) {
            return e;
        }
    }
    return NULL;
}

static TexCacheEntry* tex_cache_insert(uintptr_t data_ptr, int w, int h, u32 fmt, u32 tlut_name,
                                       uintptr_t tlut_ptr, u32 tlut_hash, u32 data_hash, GLuint gl_tex) {
    if (tex_cache_count >= TEX_CACHE_SIZE) {
        /* evict oldest half */
        int half = TEX_CACHE_SIZE / 2;
        for (int i = 0; i < half; i++) {
            if (tex_cache[i].gl_tex) {
                for (int s = 0; s < 8; s++) {
                    if (g_gx.gl_textures[s] == tex_cache[i].gl_tex)
                        g_gx.gl_textures[s] = 0;
                }
                if (!tex_cache[i].external)
                    glDeleteTextures(1, &tex_cache[i].gl_tex);
            }
        }
        memmove(&tex_cache[0], &tex_cache[half], (tex_cache_count - half) * sizeof(TexCacheEntry));
        tex_cache_count -= half;
    }
    TexCacheEntry* e = &tex_cache[tex_cache_count++];
    e->data_ptr = data_ptr;
    e->width = (u16)w;
    e->height = (u16)h;
    e->format = fmt;
    e->tlut_name = tlut_name;
    e->tlut_ptr = tlut_ptr;
    e->tlut_hash = tlut_hash;
    e->data_hash = data_hash;
    e->gl_tex = gl_tex;
    e->wrap_s = 0xFFFFFFFF;
    e->wrap_t = 0xFFFFFFFF;
    e->min_filter = 0xFFFFFFFF;
    e->external = 0;
    return e;
}

void pc_gx_texture_cache_invalidate(void) {
    for (int i = 0; i < tex_cache_count; i++) {
        if (tex_cache[i].gl_tex && !tex_cache[i].external) {
            glDeleteTextures(1, &tex_cache[i].gl_tex);
        }
    }
    tex_cache_count = 0;
}

void pc_gx_texture_init(void) {
    tex_cache_count = 0;
    tex_cache_hits = 0;
    tex_cache_misses = 0;
    (void)pc_gx_tlut_force_be();
}

void pc_gx_texture_shutdown(void) {
    pc_gx_texture_cache_invalidate();
    memset(g_gx.gl_textures, 0, sizeof(g_gx.gl_textures));
}

/* --- texture object API --- */

/* GXTexObj layout for PC: 22 u32s (88 bytes) */
#define TEXOBJ_IMAGE_PTR_LO 0
#define TEXOBJ_WIDTH       1
#define TEXOBJ_HEIGHT      2
#define TEXOBJ_FORMAT      3
#define TEXOBJ_WRAP_S      4
#define TEXOBJ_WRAP_T      5
#define TEXOBJ_MIPMAP      6
#define TEXOBJ_MIN_FILTER  7
#define TEXOBJ_MAG_FILTER  8
#define TEXOBJ_MIN_LOD     9
#define TEXOBJ_MAX_LOD     10
#define TEXOBJ_LOD_BIAS    11
#define TEXOBJ_BIAS_CLAMP  12
#define TEXOBJ_EDGE_LOD    13
#define TEXOBJ_MAX_ANISO   14
#define TEXOBJ_GL_TEX      15
#define TEXOBJ_CI_FORMAT   16
#define TEXOBJ_TLUT_NAME   17
#define TEXOBJ_IMAGE_PTR_HI 18
#define TEXOBJ_SIZE        22  /* total u32 count */

/* GXTlutObj layout (4 u32s) */
#define TLUTOBJ_DATA_LO    0
#define TLUTOBJ_FORMAT     1
#define TLUTOBJ_N_ENTRIES  2
#define TLUTOBJ_DATA_HI    3
#define TLUTOBJ_SIZE       4

static uintptr_t texobj_get_image_ptr(const u32* o) {
    uintptr_t ptr = (uintptr_t)o[TEXOBJ_IMAGE_PTR_LO];
#if UINTPTR_MAX > 0xFFFFFFFFu
    ptr |= ((uintptr_t)o[TEXOBJ_IMAGE_PTR_HI] << 32);
#endif
    return ptr;
}

static void texobj_set_image_ptr(u32* o, const void* image_ptr) {
    uintptr_t ptr = (uintptr_t)image_ptr;
    o[TEXOBJ_IMAGE_PTR_LO] = (u32)(ptr & 0xFFFFFFFFu);
#if UINTPTR_MAX > 0xFFFFFFFFu
    o[TEXOBJ_IMAGE_PTR_HI] = (u32)(ptr >> 32);
#else
    o[TEXOBJ_IMAGE_PTR_HI] = 0;
#endif
}

static uintptr_t tlutobj_get_data_ptr(const u32* o) {
    uintptr_t ptr = (uintptr_t)o[TLUTOBJ_DATA_LO];
#if UINTPTR_MAX > 0xFFFFFFFFu
    ptr |= ((uintptr_t)o[TLUTOBJ_DATA_HI] << 32);
#endif
    return ptr;
}

static void tlutobj_set_data_ptr(u32* o, const void* lut) {
    uintptr_t ptr = (uintptr_t)lut;
    o[TLUTOBJ_DATA_LO] = (u32)(ptr & 0xFFFFFFFFu);
#if UINTPTR_MAX > 0xFFFFFFFFu
    o[TLUTOBJ_DATA_HI] = (u32)(ptr >> 32);
#else
    o[TLUTOBJ_DATA_HI] = 0;
#endif
}

void GXInitTexObj(GXTexObj* obj, const void* image_ptr, u16 width, u16 height, u32 format,
                  GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, GXBool mipmap) {
    u32* o = (u32*)obj;
    memset(o, 0, TEXOBJ_SIZE * sizeof(u32));
    texobj_user_data_set(obj, NULL);
    texobj_set_image_ptr(o, image_ptr);
    o[TEXOBJ_WIDTH] = width;
    o[TEXOBJ_HEIGHT] = height;
    o[TEXOBJ_FORMAT] = format;
    o[TEXOBJ_WRAP_S] = wrap_s;
    o[TEXOBJ_WRAP_T] = wrap_t;
    o[TEXOBJ_MIPMAP] = mipmap;
    o[TEXOBJ_MIN_FILTER] = 1; /* GX_LINEAR */
    o[TEXOBJ_MAG_FILTER] = 1; /* GX_LINEAR */
}

void GXInitTexObjCI(GXTexObj* obj, const void* image_ptr, u16 width, u16 height, GXCITexFmt format,
                    GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, GXBool mipmap, u32 tlut_name) {
    GXInitTexObj(obj, image_ptr, width, height, format, wrap_s, wrap_t, mipmap);
    u32* o = (u32*)obj;
    o[TEXOBJ_CI_FORMAT] = format;
    o[TEXOBJ_TLUT_NAME] = tlut_name;
}

void GXInitTexObjData(GXTexObj* obj, const void* image_ptr) {
    u32* o = (u32*)obj;
    texobj_set_image_ptr(o, image_ptr);
}

void GXInitTexObjUserData(GXTexObj* obj, const void* user_data) {
    texobj_user_data_set(obj, user_data);
}

void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter min_filt, GXTexFilter mag_filt, f32 min_lod, f32 max_lod,
                     f32 lod_bias, GXBool bias_clamp, GXBool edge_lod, GXAnisotropy max_aniso) {
    u32* o = (u32*)obj;
    o[TEXOBJ_MIN_FILTER] = min_filt;
    o[TEXOBJ_MAG_FILTER] = mag_filt;
    /* store floats as bits */
    memcpy(&o[TEXOBJ_MIN_LOD], &min_lod, sizeof(f32));
    memcpy(&o[TEXOBJ_MAX_LOD], &max_lod, sizeof(f32));
    memcpy(&o[TEXOBJ_LOD_BIAS], &lod_bias, sizeof(f32));
    o[TEXOBJ_BIAS_CLAMP] = bias_clamp;
    o[TEXOBJ_EDGE_LOD] = edge_lod;
    o[TEXOBJ_MAX_ANISO] = max_aniso;
}

void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode s, GXTexWrapMode t) {
    u32* o = (u32*)obj;
    o[TEXOBJ_WRAP_S] = s;
    o[TEXOBJ_WRAP_T] = t;
}

/* --- GC texture format decoders (tile layout -> linear RGBA8) --- */

/* Read a big-endian u16 (GC textures store 16-bit pixels in BE order) */
static inline u16 read_be16(const u8* p) {
    return (u16)((p[0] << 8) | p[1]);
}

/* I4: 8x8 blocks, 4bpp, each byte = 2 pixels */
static void decode_I4(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 7) / 8, bh = (h + 7) / 8;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x += 2) {
                    u8 val = *src++;
                    int px0 = bx * 8 + x, py = by * 8 + y;
                    int px1 = px0 + 1;
                    u8 i0 = (val >> 4) | (val & 0xF0);
                    u8 i1 = (val & 0x0F) | ((val & 0x0F) << 4);
                    if (px0 < w && py < h) {
                        int idx = (py * w + px0) * 4;
                        dst[idx] = dst[idx+1] = dst[idx+2] = i0;
                        dst[idx+3] = 255;
                    }
                    if (px1 < w && py < h) {
                        int idx = (py * w + px1) * 4;
                        dst[idx] = dst[idx+1] = dst[idx+2] = i1;
                        dst[idx+3] = 255;
                    }
                }
            }
        }
    }
}

/* I8: 8x4 blocks, 8bpp */
static void decode_I8(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 7) / 8, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 8; x++) {
                    u8 val = *src++;
                    int px = bx * 8 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        int idx = (py * w + px) * 4;
                        dst[idx] = dst[idx+1] = dst[idx+2] = val;
                        dst[idx+3] = 255;
                    }
                }
            }
        }
    }
}

/* IA4: 8x4 blocks, high nibble = alpha, low nibble = intensity */
static void decode_IA4(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 7) / 8, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 8; x++) {
                    u8 val = *src++;
                    int px = bx * 8 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        u8 a = (val >> 4) | (val & 0xF0);
                        u8 i = (val & 0x0F) | ((val & 0x0F) << 4);
                        int idx = (py * w + px) * 4;
                        dst[idx] = dst[idx+1] = dst[idx+2] = i; dst[idx+3] = a;
                    }
                }
            }
        }
    }
}

/* A8 / CTF_A8: 8x4 blocks, 8bpp alpha */
static void decode_A8(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 7) / 8, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 8; x++) {
                    u8 a = *src++;
                    int px = bx * 8 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        int idx = (py * w + px) * 4;
                        dst[idx] = dst[idx+1] = dst[idx+2] = a;
                        dst[idx+3] = a;
                    }
                }
            }
        }
    }
}

/* IA8: 4x4 blocks, 16bpp, alpha byte + intensity byte per pixel */
static void decode_IA8(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 3) / 4, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    u8 a = *src++;
                    u8 i = *src++;
                    int px = bx * 4 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        int idx = (py * w + px) * 4;
                        dst[idx] = dst[idx+1] = dst[idx+2] = i; dst[idx+3] = a;
                    }
                }
            }
        }
    }
}

/* RGB565: 4x4 blocks, 16bpp */
static void decode_RGB565(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 3) / 4, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    u16 val = (src[0] << 8) | src[1]; src += 2;
                    int px = bx * 4 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        u8 r = ((val >> 11) & 0x1F) * 255 / 31;
                        u8 g = ((val >> 5) & 0x3F) * 255 / 63;
                        u8 b = (val & 0x1F) * 255 / 31;
                        int idx = (py * w + px) * 4;
                        dst[idx] = r; dst[idx+1] = g; dst[idx+2] = b; dst[idx+3] = 255;
                    }
                }
            }
        }
    }
}

/* RGB5A3: 4x4 blocks, 16bpp. Bit 15=1: RGB555 opaque, bit 15=0: ARGB3444 */
static void decode_RGB5A3(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 3) / 4, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    u16 val = (src[0] << 8) | src[1]; src += 2;
                    int px = bx * 4 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        int idx = (py * w + px) * 4;
                        decode_rgb5a3_entry(val, &dst[idx], &dst[idx+1], &dst[idx+2], &dst[idx+3]);
                    }
                }
            }
        }
    }
}

/* RGBA8: 4x4 blocks, 32bpp. Two passes per block: AR then GB (64 bytes total) */
static void decode_RGBA8(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 3) / 4, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            u8 ar[16][2]; /* AR pass */
            for (int i = 0; i < 16; i++) {
                ar[i][0] = *src++;
                ar[i][1] = *src++;
            }
            for (int i = 0; i < 16; i++) { /* GB pass */
                int x = i % 4, y = i / 4;
                int px = bx * 4 + x, py = by * 4 + y;
                u8 g = *src++;
                u8 b = *src++;
                if (px < w && py < h) {
                    int idx = (py * w + px) * 4;
                    dst[idx] = ar[i][1];
                    dst[idx+1] = g;
                    dst[idx+2] = b;
                    dst[idx+3] = ar[i][0];
                }
            }
        }
    }
}

/* CMPR (S3TC/DXT1): 8x8 super-blocks of 2x2 sub-blocks, each sub is 4x4 DXT1 */
static void decode_CMPR(const u8* src, u8* dst, int w, int h) {
    int bw = (w + 7) / 8, bh = (h + 7) / 8;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int sub = 0; sub < 4; sub++) {
                int sx = (sub & 1) * 4, sy = (sub >> 1) * 4;
                u16 c0 = (src[0] << 8) | src[1]; /* DXT1 block (BE) */
                u16 c1 = (src[2] << 8) | src[3];
                src += 4;
                u8 palette[4][4];
                palette[0][0] = ((c0 >> 11) & 0x1F) * 255 / 31;
                palette[0][1] = ((c0 >> 5) & 0x3F) * 255 / 63;
                palette[0][2] = (c0 & 0x1F) * 255 / 31;
                palette[0][3] = 255;
                palette[1][0] = ((c1 >> 11) & 0x1F) * 255 / 31;
                palette[1][1] = ((c1 >> 5) & 0x3F) * 255 / 63;
                palette[1][2] = (c1 & 0x1F) * 255 / 31;
                palette[1][3] = 255;
                /* interpolated colors */
                if (c0 > c1) {
                    for (int c = 0; c < 3; c++) {
                        palette[2][c] = (2 * palette[0][c] + palette[1][c]) / 3;
                        palette[3][c] = (palette[0][c] + 2 * palette[1][c]) / 3;
                    }
                    palette[2][3] = palette[3][3] = 255;
                } else {
                    for (int c = 0; c < 3; c++)
                        palette[2][c] = (palette[0][c] + palette[1][c]) / 2;
                    palette[2][3] = 255;
                    palette[3][0] = palette[3][1] = palette[3][2] = 0;
                    palette[3][3] = 0;
                }
                for (int y = 0; y < 4; y++) {
                    u8 row = *src++;
                    for (int x = 0; x < 4; x++) {
                        int ci = (row >> (6 - x * 2)) & 3;
                        int px = bx * 8 + sx + x, py = by * 8 + sy + y;
                        if (px < w && py < h) {
                            int idx = (py * w + px) * 4;
                            dst[idx] = palette[ci][0];
                            dst[idx+1] = palette[ci][1];
                            dst[idx+2] = palette[ci][2];
                            dst[idx+3] = palette[ci][3];
                        }
                    }
                }
            }
        }
    }
}

/* Decode a single RGB5A3 value to RGBA8 */
static void decode_rgb5a3_entry(u16 val, u8* r, u8* g, u8* b, u8* a) {
    if (val & 0x8000) { /* RGB555 opaque */
        *r = ((val >> 10) & 0x1F) * 255 / 31;
        *g = ((val >> 5) & 0x1F) * 255 / 31;
        *b = (val & 0x1F) * 255 / 31;
        *a = 255;
    } else { /* ARGB3444 */
        *a = ((val >> 12) & 0x07) * 255 / 7;
        *r = ((val >> 8) & 0x0F) * 255 / 15;
        *g = ((val >> 4) & 0x0F) * 255 / 15;
        *b = (val & 0x0F) * 255 / 15;
    }
}

/* Decode a single RGB565 value to RGBA8 (always opaque) */
static void decode_rgb565_entry(u16 val, u8* r, u8* g, u8* b, u8* a) {
    *r = (u8)(((val >> 11) & 0x1F) * 255 / 31);
    *g = (u8)(((val >> 5) & 0x3F) * 255 / 63);
    *b = (u8)((val & 0x1F) * 255 / 31);
    *a = 255;
}

/* Build an RGBA8 palette from TLUT data.
 * is_be=1 for ROM/JSystem data (BE), 0 for native-LE sources. */
static void build_palette(const void* tlut_data, int tlut_fmt, int n_entries,
                          u8* palette_rgba, int palette_entries, int is_be) {
    if (!palette_rgba || palette_entries <= 0) return;

    /* default ramp for missing/partial palettes */
    for (int i = 0; i < palette_entries; i++) {
        u8 v = (u8)i;
        palette_rgba[i * 4 + 0] = v;
        palette_rgba[i * 4 + 1] = v;
        palette_rgba[i * 4 + 2] = v;
        palette_rgba[i * 4 + 3] = 255;
    }

    if (!tlut_data || n_entries <= 0) return;
    if (n_entries > palette_entries) n_entries = palette_entries;

    const u16* pal16 = (const u16*)tlut_data;
    const u8* pal_bytes = (const u8*)tlut_data;
    for (int i = 0; i < n_entries; i++) {
        u16 val;
        if (is_be) {
            val = read_be16(pal_bytes + i * 2);
        } else {
            val = pal16[i];
        }

        u8* out = &palette_rgba[i * 4];
        if (tlut_fmt == GX_TL_RGB5A3) {
            decode_rgb5a3_entry(val, &out[0], &out[1], &out[2], &out[3]);
        } else if (tlut_fmt == GX_TL_RGB565) {
            decode_rgb565_entry(val, &out[0], &out[1], &out[2], &out[3]);
        } else { /* GX_TL_IA8: high byte = intensity, low byte = alpha */
            if (is_be) {
                out[0] = out[1] = out[2] = (u8)(val >> 8);
                out[3] = (u8)(val & 0xFF);
            } else {
                /* LE read of BE bytes [I,A] swaps them */
                out[0] = out[1] = out[2] = (u8)(val & 0xFF);
                out[3] = (u8)(val >> 8);
            }
        }
    }
}

/* CI4: 8x8 blocks, 4bpp indexed */
static void decode_CI4(const u8* src, u8* dst, int w, int h, const u8* palette) {
    int bw = (w + 7) / 8, bh = (h + 7) / 8;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x += 2) {
                    u8 val = *src++;
                    int px0 = bx * 8 + x, py = by * 8 + y;
                    int px1 = px0 + 1;
                    int ci0 = (val >> 4) & 0xF;
                    int ci1 = val & 0xF;
                    if (px0 < w && py < h) {
                        int idx = (py * w + px0) * 4;
                        const u8* p = &palette[ci0 * 4];
                        dst[idx] = p[0]; dst[idx+1] = p[1];
                        dst[idx+2] = p[2]; dst[idx+3] = p[3];
                    }
                    if (px1 < w && py < h) {
                        int idx = (py * w + px1) * 4;
                        const u8* p = &palette[ci1 * 4];
                        dst[idx] = p[0]; dst[idx+1] = p[1];
                        dst[idx+2] = p[2]; dst[idx+3] = p[3];
                    }
                }
            }
        }
    }
}

/* CI8: 8x4 blocks, 8bpp indexed */
static void decode_CI8(const u8* src, u8* dst, int w, int h, const u8* palette) {
    int bw = (w + 7) / 8, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 8; x++) {
                    u8 val = *src++;
                    int px = bx * 8 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        int idx = (py * w + px) * 4;
                        const u8* p = &palette[(int)val * 4];
                        dst[idx] = p[0]; dst[idx+1] = p[1];
                        dst[idx+2] = p[2]; dst[idx+3] = p[3];
                    }
                }
            }
        }
    }
}

/* CI14X2: 4x4 blocks, 16bpp indexed (lower 14 bits are palette index) */
static void decode_CI14X2(const u8* src, u8* dst, int w, int h, const u8* palette, int palette_entries) {
    int bw = (w + 3) / 4, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) {
        for (int bx = 0; bx < bw; bx++) {
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++) {
                    u16 raw = read_be16(src);
                    src += 2;
                    int ci = (int)(raw & 0x3FFFu);
                    int px = bx * 4 + x, py = by * 4 + y;
                    if (px < w && py < h) {
                        int idx = (py * w + px) * 4;
                        if (ci >= 0 && ci < palette_entries) {
                            const u8* p = &palette[ci * 4];
                            dst[idx] = p[0]; dst[idx+1] = p[1];
                            dst[idx+2] = p[2]; dst[idx+3] = p[3];
                        } else {
                            dst[idx] = dst[idx+1] = dst[idx+2] = 0;
                            dst[idx+3] = 255;
                        }
                    }
                }
            }
        }
    }
}

static void decode_gc_texture(const void* src, u8* dst_rgba, int w, int h, u32 fmt,
                              const u8* palette, int palette_entries) {
    /* transparent fallback for unhandled formats */
    memset(dst_rgba, 0, w * h * 4);

    switch (fmt) {
        case GX_TF_I4:     decode_I4((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_I8:     decode_I8((const u8*)src, dst_rgba, w, h); break;
        case GX_CTF_A8:    decode_A8((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_IA4:    decode_IA4((const u8*)src, dst_rgba, w, h); break;
        case GX_CTF_RA4:   decode_IA4((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_IA8:    decode_IA8((const u8*)src, dst_rgba, w, h); break;
        case GX_CTF_RA8:
        case GX_CTF_RG8:
        case GX_CTF_GB8:
        case GX_TF_Z16:
        case GX_CTF_Z16L:  decode_IA8((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_RGB565: decode_RGB565((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_RGB5A3: decode_RGB5A3((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_RGBA8:  decode_RGBA8((const u8*)src, dst_rgba, w, h); break;
        case GX_CTF_YUVA8: decode_RGBA8((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_Z24X8:  decode_RGBA8((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_C4:     decode_CI4((const u8*)src, dst_rgba, w, h, palette); break;
        case GX_TF_C8:     decode_CI8((const u8*)src, dst_rgba, w, h, palette); break;
        case GX_TF_C14X2:  decode_CI14X2((const u8*)src, dst_rgba, w, h, palette, palette_entries); break;
        case GX_CTF_R4:
        case GX_CTF_Z4:    decode_I4((const u8*)src, dst_rgba, w, h); break;
        case GX_CTF_R8:
        case GX_CTF_G8:
        case GX_CTF_B8:
        case GX_TF_Z8:
        case GX_CTF_Z8M:
        case GX_CTF_Z8L:   decode_I8((const u8*)src, dst_rgba, w, h); break;
        case GX_TF_CMPR:   decode_CMPR((const u8*)src, dst_rgba, w, h); break;
        default:
            break;
    }
}

void GXLoadTexObj(GXTexObj* obj, GXTexMapID id) {
    pc_gx_flush_if_begin_complete();

    if (id >= 8 && id != 0xFF && id < 0x100) return;
    if (id >= 8) return;

    u32* o = (u32*)obj;
    uintptr_t image_ptr_key = texobj_get_image_ptr(o);
    void* image_ptr = (void*)image_ptr_key;
    int width = (int)o[TEXOBJ_WIDTH];
    int height = (int)o[TEXOBJ_HEIGHT];
    u32 format = o[TEXOBJ_FORMAT];
    u32 wrap_s = o[TEXOBJ_WRAP_S], wrap_t = o[TEXOBJ_WRAP_T];
    u32 tlut_key = (format == GX_TF_C4 || format == GX_TF_C8 || format == GX_TF_C14X2) ? o[TEXOBJ_TLUT_NAME] : 0xFFFFFFFF;
    uintptr_t tlut_ptr_key = 0;
    u32 tlut_hash_key = 0;
    u32 filter_mode = o[TEXOBJ_MIN_FILTER];

    if (!s_skip_region_callbacks && s_tex_region_callback) {
        (void)s_tex_region_callback((GXTexObj*)obj, (GXTexMapID)id);
    }
    if (!s_skip_region_callbacks &&
        (format == GX_TF_C4 || format == GX_TF_C8 || format == GX_TF_C14X2) &&
        s_tlut_region_callback) {
        (void)s_tlut_region_callback(o[TEXOBJ_TLUT_NAME]);
    }

    if (format == GX_TF_C4 || format == GX_TF_C8 || format == GX_TF_C14X2) {
        int tlut_name = (int)o[TEXOBJ_TLUT_NAME];
        if (tlut_name >= 0 && tlut_name < PC_GX_TLUT_SLOTS && g_gx.tlut[tlut_name].data) {
            tlut_ptr_key = (uintptr_t)g_gx.tlut[tlut_name].data;
            tlut_hash_key = tlut_content_hash(g_gx.tlut[tlut_name].data,
                                              g_gx.tlut[tlut_name].format,
                                              g_gx.tlut[tlut_name].n_entries,
                                              g_gx.tlut[tlut_name].is_be);
        }
    }

#ifdef PC_ENHANCEMENTS
    /* EFB capture bypass: use full-res FBO texture instead of re-decoding */
    {
        GLuint efb_tex = pc_gx_efb_capture_find(image_ptr_key);
        int is_depth_copy_fmt =
            (format == GX_TF_Z24X8) ||
            (format == GX_TF_Z16) ||
            (format == GX_CTF_Z16L) ||
            (format == GX_TF_Z8) ||
            (format == GX_CTF_Z8M) ||
            (format == GX_CTF_Z8L) ||
            (format == GX_CTF_Z4);

        if (efb_tex && !is_depth_copy_fmt) {
            glBindTexture(GL_TEXTURE_2D, efb_tex);
            GLenum gl_filter = filter_mode ? GL_LINEAR : GL_NEAREST;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
            o[TEXOBJ_GL_TEX] = efb_tex;
            g_gx.gl_textures[id] = efb_tex;
            g_gx.tex_obj_w[id] = width;
            g_gx.tex_obj_h[id] = height;
            g_gx.tex_obj_fmt[id] = (int)format;
            DIRTY(PC_GX_DIRTY_TEXTURES);
            return;
        }
    }
#endif

    /* detect when emu64 reuses the same buffer with different data */
    u32 hash = tex_content_hash(image_ptr, width, height, format);

    /* cache lookup */
    TexCacheEntry* cached = tex_cache_find(image_ptr_key, width, height, format, tlut_key,
                                           tlut_ptr_key, tlut_hash_key, hash);
    if (cached) {
        tex_cache_hits++;
        GLuint tex = cached->gl_tex;
        glBindTexture(GL_TEXTURE_2D, tex);

        /* update wrap/filter if changed */
        if (cached->wrap_s != wrap_s || cached->wrap_t != wrap_t) {
            GLenum gl_ws = (wrap_s == 2) ? GL_MIRRORED_REPEAT :
                           (wrap_s == 0) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
            GLenum gl_wt = (wrap_t == 2) ? GL_MIRRORED_REPEAT :
                           (wrap_t == 0) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_ws);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wt);
            cached->wrap_s = wrap_s;
            cached->wrap_t = wrap_t;
        }
        if (cached->min_filter != filter_mode) {
            GLenum gl_filter = filter_mode ? GL_LINEAR : GL_NEAREST;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
            cached->min_filter = filter_mode;
        }

        o[TEXOBJ_GL_TEX] = tex;
        g_gx.gl_textures[id] = tex;
        g_gx.tex_obj_w[id] = width;
        g_gx.tex_obj_h[id] = height;
        g_gx.tex_obj_fmt[id] = (int)format;
        DIRTY(PC_GX_DIRTY_TEXTURES);
        return;
    }

    /* cache miss */
    tex_cache_misses++;

    /* try texture pack replacement before decoding */
    if (pc_texture_pack_active()) {
        const void* tp_tlut = NULL;
        int tp_tlut_entries = 0;
        int tp_tlut_is_be = 1;
        if ((format == GX_TF_C4 || format == GX_TF_C8 || format == GX_TF_C14X2)) {
            int tlut_name = (int)o[TEXOBJ_TLUT_NAME];
            if (tlut_name >= 0 && tlut_name < PC_GX_TLUT_SLOTS && g_gx.tlut[tlut_name].data) {
                tp_tlut = g_gx.tlut[tlut_name].data;
                tp_tlut_entries = g_gx.tlut[tlut_name].n_entries;
                tp_tlut_is_be = g_gx.tlut[tlut_name].is_be;
            }
        }
        int data_size = (width * height * gc_format_bpp(format)) / 8;
        int hd_w = 0, hd_h = 0;
        GLuint hd_tex = pc_texture_pack_lookup(image_ptr, data_size,
                                               width, height, format,
                                               tp_tlut, tp_tlut_entries, tp_tlut_is_be,
                                               &hd_w, &hd_h);
        if (hd_tex) {
            glBindTexture(GL_TEXTURE_2D, hd_tex);
            GLenum gl_ws = (wrap_s == 2) ? GL_MIRRORED_REPEAT :
                           (wrap_s == 0) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
            GLenum gl_wt = (wrap_t == 2) ? GL_MIRRORED_REPEAT :
                           (wrap_t == 0) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_ws);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wt);

            TexCacheEntry* entry = tex_cache_insert(image_ptr_key, width, height, format,
                                                    tlut_key, tlut_ptr_key, tlut_hash_key, hash, hd_tex);
            entry->wrap_s = wrap_s;
            entry->wrap_t = wrap_t;
            entry->min_filter = filter_mode;
            entry->external = 1;

            o[TEXOBJ_GL_TEX] = hd_tex;
            g_gx.gl_textures[id] = hd_tex;
            g_gx.tex_obj_w[id] = width;
            g_gx.tex_obj_h[id] = height;
            g_gx.tex_obj_fmt[id] = (int)format;
            DIRTY(PC_GX_DIRTY_TEXTURES);
            return;
        }
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    if (image_ptr && width > 0 && height > 0 && width <= 1024 && height <= 1024) {
        u8* rgba = (u8*)malloc(width * height * 4);
        if (rgba) {
            int palette_entries = (format == GX_TF_C14X2) ? 16384 : 256;
            u8* palette = (u8*)malloc((size_t)palette_entries * 4u);
            if (!palette) {
                u8 white[4] = {255, 255, 255, 255};
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
                free(rgba);
            } else {
                if (format == GX_TF_C4 || format == GX_TF_C8 || format == GX_TF_C14X2) {
                    int tlut_name = (int)o[TEXOBJ_TLUT_NAME];
                    if (tlut_name >= 0 && tlut_name < PC_GX_TLUT_SLOTS && g_gx.tlut[tlut_name].data) {
                        build_palette(g_gx.tlut[tlut_name].data,
                                      g_gx.tlut[tlut_name].format,
                                      g_gx.tlut[tlut_name].n_entries,
                                      palette,
                                      palette_entries,
                                      g_gx.tlut[tlut_name].is_be);
                    } else {
                        build_palette(NULL, 0, 0, palette, palette_entries, 0);
                    }
                } else {
                    build_palette(NULL, 0, 0, palette, palette_entries, 0);
                }

                decode_gc_texture(image_ptr, rgba, width, height, format, palette, palette_entries);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE, rgba);
                PC_GL_CHECK("glTexImage2D");
                free(palette);
                free(rgba);
            }
        } else {
            u8 white[4] = {255, 255, 255, 255};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
        }
    } else {
        u8 white[4] = {255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    }

    {
        GLenum gl_filter = filter_mode ? GL_LINEAR : GL_NEAREST;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter);
    }

    {
        GLenum gl_ws = (wrap_s == 2) ? GL_MIRRORED_REPEAT :
                       (wrap_s == 0) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
        GLenum gl_wt = (wrap_t == 2) ? GL_MIRRORED_REPEAT :
                       (wrap_t == 0) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_ws);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wt);
    }

    /* insert into cache */
    TexCacheEntry* entry = tex_cache_insert(image_ptr_key, width, height, format, tlut_key,
                                            tlut_ptr_key, tlut_hash_key, hash, tex);
    entry->wrap_s = wrap_s;
    entry->wrap_t = wrap_t;
    entry->min_filter = filter_mode;

    o[TEXOBJ_GL_TEX] = tex;
    g_gx.gl_textures[id] = tex;
    g_gx.tex_obj_w[id] = width;
    g_gx.tex_obj_h[id] = height;
    g_gx.tex_obj_fmt[id] = (int)format;
    DIRTY(PC_GX_DIRTY_TEXTURES);
}

u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, GXBool mipmap, u8 max_lod) {
    s32 shift_x = 0;
    s32 shift_y = 0;
    u32 block_bytes = 32;
    u32 total = 0;

    switch (format) {
        case GX_TF_I4:
        case GX_TF_C4:
        case GX_TF_CMPR:
        case GX_CTF_R4:
        case GX_CTF_Z4:
            shift_x = 3;
            shift_y = 3;
            break;
        case GX_TF_I8:
        case GX_TF_IA4:
        case GX_TF_C8:
        case GX_TF_Z8:
        case GX_CTF_RA4:
        case GX_CTF_A8:
        case GX_CTF_R8:
        case GX_CTF_G8:
        case GX_CTF_B8:
        case GX_CTF_Z8M:
        case GX_CTF_Z8L:
            shift_x = 3;
            shift_y = 2;
            break;
        case GX_TF_IA8:
        case GX_TF_RGB565:
        case GX_TF_RGB5A3:
        case GX_TF_RGBA8:
        case GX_TF_C14X2:
        case GX_TF_Z16:
        case GX_TF_Z24X8:
        case GX_CTF_RA8:
        case GX_CTF_YUVA8:
        case GX_CTF_RG8:
        case GX_CTF_GB8:
        case GX_CTF_Z16L:
            shift_x = 2;
            shift_y = 2;
            break;
        default:
            shift_x = 0;
            shift_y = 0;
            break;
    }

    if (format == GX_TF_RGBA8 || format == GX_TF_Z24X8 || format == GX_CTF_YUVA8) {
        block_bytes = 64;
    }

    if (mipmap) {
        while (max_lod != 0) {
            u32 tile_x = ((u32)width + (1u << shift_x) - 1u) >> shift_x;
            u32 tile_y = ((u32)height + (1u << shift_y) - 1u) >> shift_y;
            total += block_bytes * tile_x * tile_y;

            if (width == 1 && height == 1) {
                return total;
            }

            width = (width < 2) ? 1 : (u16)(width / 2);
            height = (height < 2) ? 1 : (u16)(height / 2);
            --max_lod;
        }
    } else {
        u32 tile_x = ((u32)width + (1u << shift_x) - 1u) >> shift_x;
        u32 tile_y = ((u32)height + (1u << shift_y) - 1u) >> shift_y;
        total = block_bytes * tile_x * tile_y;
    }

    return total;
}

void GXInvalidateTexAll(void) {
    /* no-op: PC cache keys on pointer+format, no TMEM to flush */
}
void GXInvalidateTexRegion(const GXTexRegion* region) { (void)region; }

/* --- TLUT --- */

void GXInitTlutObj(GXTlutObj* obj, const void* lut, GXTlutFmt fmt, u16 n_entries) {
    u32* o = (u32*)obj;
    memset(o, 0, TLUTOBJ_SIZE * sizeof(u32));
    tlutobj_set_data_ptr(o, lut);
    o[TLUTOBJ_FORMAT] = fmt;
    o[TLUTOBJ_N_ENTRIES] = n_entries;
}

void GXLoadTlut(const GXTlutObj* obj, GXTlut idx) {
    pc_gx_flush_if_begin_complete();
    if (idx >= PC_GX_TLUT_SLOTS) return;
    if (!s_skip_region_callbacks && s_tlut_region_callback) {
        (void)s_tlut_region_callback(idx);
    }
    const u32* o = (const u32*)obj;
    g_gx.tlut[idx].data = (const void*)tlutobj_get_data_ptr(o);
    g_gx.tlut[idx].format = (int)o[TLUTOBJ_FORMAT];
    g_gx.tlut[idx].n_entries = (int)o[TLUTOBJ_N_ENTRIES];
    g_gx.tlut[idx].is_be = 1; /* default to BE (ROM/JSystem data) */
}

/* PC_GX_TLUT_MODE env var: "be" forces BE decode for diagnostics */
static int pc_gx_tlut_force_be(void) {
    static int init = 0;
    static int force_be = 0;
    if (!init) {
        const char* mode = getenv("PC_GX_TLUT_MODE");
        if (mode != NULL) {
            if (mode[0] == 'b' || mode[0] == 'B') {
                force_be = 1;
            }
        }
        init = 1;
    }
    return force_be;
}

/* Mark a TLUT slot as native-LE (from emu64 tlutconv) */
void pc_gx_tlut_set_native_le(unsigned int idx) {
    if (idx < PC_GX_TLUT_SLOTS) {
        if (pc_gx_tlut_force_be()) {
            g_gx.tlut[idx].is_be = 1;
        } else {
            g_gx.tlut[idx].is_be = 0;
        }
    }
}

void GXInitTexCacheRegion(GXTexRegion* region, GXBool is_32b, u32 tmem_even, GXTexCacheSize size_even,
                          u32 tmem_odd, GXTexCacheSize size_odd) {
    u32* r = (u32*)region;
    if (!r) return;
    memset(r, 0, 8 * sizeof(u32));
    r[0] = (u32)is_32b;
    r[1] = tmem_even;
    r[2] = size_even;
    r[3] = tmem_odd;
    r[4] = size_odd;
}

GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback callback) {
    GXTexRegionCallback prev = s_tex_region_callback;
    s_tex_region_callback = callback ? callback : default_tex_region_callback;
    return prev;
}

GXTlutRegionCallback GXSetTlutRegionCallBack(GXTlutRegionCallback callback) {
    GXTlutRegionCallback prev = s_tlut_region_callback;
    s_tlut_region_callback = callback ? callback : default_tlut_region_callback;
    return prev;
}

void GXInitTexPreLoadRegion(GXTexRegion* region, u32 tmem_even, u32 size_even, u32 tmem_odd, u32 size_odd) {
    u32* r = (u32*)region;
    if (!r) return;
    memset(r, 0, 8 * sizeof(u32));
    r[0] = tmem_even;
    r[1] = size_even;
    r[2] = tmem_odd;
    r[3] = size_odd;
}

void GXInitTlutRegion(GXTlutRegion* region, u32 tmem_addr, u32 tlut_size) {
    u32* r = (u32*)region;
    if (!r) return;
    memset(r, 0, 4 * sizeof(u32));
    r[0] = tmem_addr;
    r[1] = tlut_size;
}

void GXPreLoadEntireTexture(GXTexObj* tex_obj, GXTexRegion* region) {
    (void)region;
    if (!tex_obj) return;

    GLuint prev_tex = g_gx.gl_textures[0];
    int prev_w = g_gx.tex_obj_w[0];
    int prev_h = g_gx.tex_obj_h[0];
    int prev_fmt = g_gx.tex_obj_fmt[0];

    s_skip_region_callbacks++;
    GXLoadTexObj(tex_obj, 0);
    s_skip_region_callbacks--;

    g_gx.gl_textures[0] = prev_tex;
    g_gx.tex_obj_w[0] = prev_w;
    g_gx.tex_obj_h[0] = prev_h;
    g_gx.tex_obj_fmt[0] = prev_fmt;
}

void GXLoadTexObjPreLoaded(GXTexObj* obj, GXTexRegion* region, GXTexMapID id) {
    (void)region;
    s_skip_region_callbacks++;
    GXLoadTexObj(obj, id);
    s_skip_region_callbacks--;
}

/* --- accessors --- */
GXBool GXGetTexObjMipMap(GXTexObj* obj) { return ((const u32*)obj)[TEXOBJ_MIPMAP] != 0; }
GXTexFmt GXGetTexObjFmt(GXTexObj* obj) { return (GXTexFmt)((const u32*)obj)[TEXOBJ_FORMAT]; }
u16 GXGetTexObjHeight(GXTexObj* obj) { return (u16)((const u32*)obj)[TEXOBJ_HEIGHT]; }
u16 GXGetTexObjWidth(GXTexObj* obj) { return (u16)((const u32*)obj)[TEXOBJ_WIDTH]; }
GXTexWrapMode GXGetTexObjWrapS(GXTexObj* obj) { return (GXTexWrapMode)((const u32*)obj)[TEXOBJ_WRAP_S]; }
GXTexWrapMode GXGetTexObjWrapT(GXTexObj* obj) { return (GXTexWrapMode)((const u32*)obj)[TEXOBJ_WRAP_T]; }
void* GXGetTexObjData(GXTexObj* obj) { return (void*)texobj_get_image_ptr((const u32*)obj); }
void* GXGetTexObjUserData(GXTexObj* obj) { return (void*)texobj_user_data_get(obj); }

void GXGetTexObjAll(GXTexObj* obj, void** image_ptr, u16* width, u16* height, GXTexFmt* format,
                    GXTexWrapMode* wrap_s, GXTexWrapMode* wrap_t, GXBool* mipmap) {
    const u32* o = (const u32*)obj;
    if (image_ptr) *image_ptr = (void*)texobj_get_image_ptr(o);
    if (width) *width = (u16)o[TEXOBJ_WIDTH];
    if (height) *height = (u16)o[TEXOBJ_HEIGHT];
    if (format) *format = (GXTexFmt)o[TEXOBJ_FORMAT];
    if (wrap_s) *wrap_s = (GXTexWrapMode)o[TEXOBJ_WRAP_S];
    if (wrap_t) *wrap_t = (GXTexWrapMode)o[TEXOBJ_WRAP_T];
    if (mipmap) *mipmap = (o[TEXOBJ_MIPMAP] != 0) ? GX_TRUE : GX_FALSE;
}

void* GXGetTlutObjData(GXTlutObj* obj) {
    return (void*)tlutobj_get_data_ptr((const u32*)obj);
}

GXTlutFmt GXGetTlutObjFmt(GXTlutObj* obj) {
    return (GXTlutFmt)((const u32*)obj)[TLUTOBJ_FORMAT];
}

u16 GXGetTlutObjNumEntries(GXTlutObj* obj) {
    return (u16)((const u32*)obj)[TLUTOBJ_N_ENTRIES];
}

void GXGetTlutObjAll(GXTlutObj* obj, void** lut, GXTlutFmt* fmt, u16* n_entries) {
    const u32* o = (const u32*)obj;
    if (lut) *lut = (void*)tlutobj_get_data_ptr(o);
    if (fmt) *fmt = (GXTlutFmt)o[TLUTOBJ_FORMAT];
    if (n_entries) *n_entries = (u16)o[TLUTOBJ_N_ENTRIES];
}

void GXDestroyTexObj(void* obj) {
    u32* o = (u32*)obj;
    /* don't delete GL texture here; cache eviction handles that */
    o[TEXOBJ_GL_TEX] = 0;
}

void GXDestroyTlutObj(void* obj) { (void)obj; }
