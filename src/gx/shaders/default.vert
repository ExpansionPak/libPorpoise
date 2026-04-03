#version 330 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_color0;
layout(location = 3) in vec2 a_texcoord0;
layout(location = 5) in vec2 a_texcoord1;
layout(location = 6) in vec2 a_texcoord2;
layout(location = 7) in vec2 a_texcoord3;
layout(location = 4) in float a_pn_mtx_idx;
uniform mat4 u_projection;
uniform mat4 u_modelview;
uniform mat3 u_normal_mtx;
uniform mat4 u_modelview_arr[10];
uniform mat3 u_normal_mtx_arr[10];
uniform int u_use_matrix_array;
uniform vec4 u_texmtx_row0[4];
uniform vec4 u_texmtx_row1[4];
uniform int u_texmtx_enable[4];
uniform int u_texgen_src[4];  /* GX_TG_* source enum */
uniform int u_texgen_type[4]; /* GX_TG_* func enum */
uniform int u_texgen_normalize[4];
out vec4 v_color;
out vec2 v_texcoord0;
out vec2 v_texcoord1;
out vec2 v_texcoord2;
out vec2 v_texcoord3;
out vec3 v_normal;
out vec3 v_eye_pos;
out float v_fog_z;

vec4 texgenInput(int src, vec3 pos, vec3 nrm, vec2 tex0, vec2 tex1, vec2 tex2, vec2 tex3,
                 vec2 gen0, vec2 gen1, vec2 gen2, vec2 gen3, vec4 color0) {
    /* GXTexGenSrc enum mapping.
       BINRM/TANGENT and TEX4-7 are fallbacked due to missing attributes in PC vertex format. */
    if (src == 0) return vec4(pos, 1.0);                  /* GX_TG_POS */
    if (src == 1) return vec4(nrm, 1.0);                  /* GX_TG_NRM */
    if (src == 2) return vec4(0.0, 0.0, 1.0, 1.0);        /* GX_TG_BINRM (fallback) */
    if (src == 3) return vec4(0.0, 0.0, 1.0, 1.0);        /* GX_TG_TANGENT (fallback) */
    if (src == 4) return vec4(tex0, 0.0, 1.0);            /* GX_TG_TEX0 */
    if (src == 5) return vec4(tex1, 0.0, 1.0);            /* GX_TG_TEX1 */
    if (src == 6) return vec4(tex2, 0.0, 1.0);            /* GX_TG_TEX2 */
    if (src == 7) return vec4(tex3, 0.0, 1.0);            /* GX_TG_TEX3 */
    if (src >= 8 && src <= 11) return vec4(tex0, 0.0, 1.0); /* GX_TG_TEX4..7 fallback */
    if (src == 12) return vec4(gen0, 0.0, 1.0);           /* GX_TG_TEXCOORD0 */
    if (src == 13) return vec4(gen1, 0.0, 1.0);           /* GX_TG_TEXCOORD1 */
    if (src == 14) return vec4(gen2, 0.0, 1.0);           /* GX_TG_TEXCOORD2 */
    if (src == 15) return vec4(gen3, 0.0, 1.0);           /* GX_TG_TEXCOORD3 */
    if (src >= 16 && src <= 18) return vec4(gen0, 0.0, 1.0); /* GX_TG_TEXCOORD4..6 fallback */
    if (src == 19 || src == 20) return vec4(color0.rg, 0.0, 1.0); /* GX_TG_COLOR0/1 */
    return vec4(tex0, 0.0, 1.0);
}

vec2 applyTexGen(int idx, vec3 pos, vec3 nrm, vec2 tex0, vec2 tex1, vec2 tex2, vec2 tex3,
                 vec2 gen0, vec2 gen1, vec2 gen2, vec2 gen3, vec4 color0) {
    int func = u_texgen_type[idx];
    vec4 tc = texgenInput(u_texgen_src[idx], pos, nrm, tex0, tex1, tex2, tex3,
                          gen0, gen1, gen2, gen3, color0);

    /* GX_TG_SRTG uses rasterized channel color to produce s/t. */
    if (func == 10) {
        return clamp(tc.xy, 0.0, 1.0);
    }

    /* GX_TG_BUMP0..7 are currently pass-through (emboss perturbation is not yet emulated). */
    if (func >= 2 && func <= 9) {
        return tc.xy;
    }

    vec2 outTc = tc.xy;
    if (u_texmtx_enable[idx] != 0) {
        outTc = vec2(dot(u_texmtx_row0[idx], tc), dot(u_texmtx_row1[idx], tc));
    }

    /* HW2 normalize option for ordinary texgens (GX_TG_MTX3x4 / GX_TG_MTX2x4). */
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
    v_eye_pos = eyePos.xyz;
    v_fog_z = -eyePos.z;
    v_color = a_color0;
    v_normal = normalize(nm * a_normal);

    vec3 srcNrm = normalize(a_normal);
    vec2 gen0 = applyTexGen(0, a_position, srcNrm, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3,
                            vec2(0.0), vec2(0.0), vec2(0.0), vec2(0.0), a_color0);
    vec2 gen1 = applyTexGen(1, a_position, srcNrm, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3,
                            gen0, vec2(0.0), vec2(0.0), vec2(0.0), a_color0);
    vec2 gen2 = applyTexGen(2, a_position, srcNrm, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3,
                            gen0, gen1, vec2(0.0), vec2(0.0), a_color0);
    vec2 gen3 = applyTexGen(3, a_position, srcNrm, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3,
                            gen0, gen1, gen2, vec2(0.0), a_color0);

    v_texcoord0 = gen0;
    v_texcoord1 = gen1;
    v_texcoord2 = gen2;
    v_texcoord3 = gen3;
}
