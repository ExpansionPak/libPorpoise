#version 330 core
#define PC_GX_MAX_TEXGENS 8

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_color0;
layout(location = 3) in vec2 a_texcoord0;
layout(location = 5) in vec2 a_texcoord1;
layout(location = 6) in vec2 a_texcoord2;
layout(location = 7) in vec2 a_texcoord3;
layout(location = 8) in vec2 a_texcoord4;
layout(location = 9) in vec2 a_texcoord5;
layout(location = 10) in vec2 a_texcoord6;
layout(location = 11) in vec2 a_texcoord7;
layout(location = 4) in float a_pn_mtx_idx;

uniform mat4 u_projection;
uniform mat4 u_modelview;
uniform mat3 u_normal_mtx;
uniform mat4 u_modelview_arr[10];
uniform mat3 u_normal_mtx_arr[10];
uniform int u_use_matrix_array;
uniform vec4 u_texmtx_row0[PC_GX_MAX_TEXGENS];
uniform vec4 u_texmtx_row1[PC_GX_MAX_TEXGENS];
uniform int u_texmtx_enable[PC_GX_MAX_TEXGENS];
uniform int u_texgen_src[PC_GX_MAX_TEXGENS];
uniform int u_texgen_type[PC_GX_MAX_TEXGENS];
uniform int u_texgen_normalize[PC_GX_MAX_TEXGENS];

out vec4 v_color;
out vec2 v_texcoord0;
out vec2 v_texcoord1;
out vec2 v_texcoord2;
out vec2 v_texcoord3;
out vec2 v_texcoord4;
out vec2 v_texcoord5;
out vec2 v_texcoord6;
out vec2 v_texcoord7;
out vec3 v_normal;
out vec3 v_eye_pos;
out float v_fog_z;

vec4 texgenInput(int src, vec3 pos, vec3 nrm, vec2 tex[PC_GX_MAX_TEXGENS], vec2 gen[PC_GX_MAX_TEXGENS], vec4 color0) {
    if (src == 0) return vec4(pos, 1.0);             /* GX_TG_POS */
    if (src == 1) return vec4(nrm, 1.0);             /* GX_TG_NRM */
    if (src == 2) return vec4(0.0, 0.0, 1.0, 1.0);   /* GX_TG_BINRM (fallback) */
    if (src == 3) return vec4(0.0, 0.0, 1.0, 1.0);   /* GX_TG_TANGENT (fallback) */
    if (src >= 4 && src <= 11) return vec4(tex[src - 4], 0.0, 1.0);   /* GX_TG_TEX0..7 */
    if (src >= 12 && src <= 18) return vec4(gen[src - 12], 0.0, 1.0); /* GX_TG_TEXCOORD0..6 */
    if (src == 19 || src == 20) return vec4(color0.rg, 0.0, 1.0);     /* GX_TG_COLOR0/1 */
    return vec4(tex[0], 0.0, 1.0);
}

vec2 applyTexGen(int idx, vec3 pos, vec3 nrm, vec2 tex[PC_GX_MAX_TEXGENS], vec2 gen[PC_GX_MAX_TEXGENS], vec4 color0) {
    int func = u_texgen_type[idx];
    vec4 tc = texgenInput(u_texgen_src[idx], pos, nrm, tex, gen, color0);

    /* GX_TG_SRTG uses rasterized channel color to produce s/t. */
    if (func == 10) {
        return clamp(tc.xy, 0.0, 1.0);
    }

    /* GX_TG_BUMP0..7 are pass-through for now. */
    if (func >= 2 && func <= 9) {
        return tc.xy;
    }

    vec2 outTc = tc.xy;
    if (u_texmtx_enable[idx] != 0) {
        outTc = vec2(dot(u_texmtx_row0[idx], tc), dot(u_texmtx_row1[idx], tc));
    }

    if (u_texgen_normalize[idx] != 0 && (func == 0 || func == 1)) {
        float lenTc = length(outTc);
        if (lenTc > 1e-8) {
            outTc /= lenTc;
        }
    }
    return outTc;
}

void main() {
    mat4 mv = u_modelview;
    mat3 nm = u_normal_mtx;
    if (u_use_matrix_array != 0) {
        int midx = clamp(int(a_pn_mtx_idx + 0.5), 0, 9);
        mv = u_modelview_arr[midx];
        nm = u_normal_mtx_arr[midx];
    }

    vec4 eyePos = mv * vec4(a_position, 1.0);
    gl_Position = u_projection * eyePos;
    /*
     * GX projection matrices produce clip-space Z in [0,w].
     * OpenGL expects clip-space Z in [-w,w], so remap with z' = 2z - w.
     */
    gl_Position.z = (2.0 * gl_Position.z) - gl_Position.w;
    v_eye_pos = eyePos.xyz;
    v_fog_z = -eyePos.z;
    v_color = a_color0;
    v_normal = normalize(nm * a_normal);

    vec2 srcTex[PC_GX_MAX_TEXGENS];
    srcTex[0] = a_texcoord0;
    srcTex[1] = a_texcoord1;
    srcTex[2] = a_texcoord2;
    srcTex[3] = a_texcoord3;
    srcTex[4] = a_texcoord4;
    srcTex[5] = a_texcoord5;
    srcTex[6] = a_texcoord6;
    srcTex[7] = a_texcoord7;

    vec2 gen[PC_GX_MAX_TEXGENS];
    for (int i = 0; i < PC_GX_MAX_TEXGENS; ++i) {
        gen[i] = vec2(0.0);
    }

    vec3 srcNrm = normalize(a_normal);
    for (int i = 0; i < PC_GX_MAX_TEXGENS; ++i) {
        gen[i] = applyTexGen(i, a_position, srcNrm, srcTex, gen, a_color0);
    }

    v_texcoord0 = gen[0];
    v_texcoord1 = gen[1];
    v_texcoord2 = gen[2];
    v_texcoord3 = gen[3];
    v_texcoord4 = gen[4];
    v_texcoord5 = gen[5];
    v_texcoord6 = gen[6];
    v_texcoord7 = gen[7];
}
