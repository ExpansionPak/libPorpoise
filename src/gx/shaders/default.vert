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
out vec4 v_color;
out vec2 v_texcoord0;
out vec2 v_texcoord1;
out vec2 v_texcoord2;
out vec2 v_texcoord3;
out vec3 v_normal;
out vec3 v_eye_pos;
out float v_fog_z;

vec4 texgenInput(int src, vec3 pos, vec3 nrm, vec2 tex0, vec2 tex1, vec2 tex2, vec2 tex3) {
    /* GX_TG_POS = 0, GX_TG_NRM = 1, GX_TG_TEX0..3 = 4..7, GX_TG_TEXCOORD0..3 = 12..15. */
    if (src == 0) return vec4(pos, 1.0);
    if (src == 1) return vec4(nrm, 1.0);
    if (src == 5 || src == 13) return vec4(tex1, 0.0, 1.0);
    if (src == 6 || src == 14) return vec4(tex2, 0.0, 1.0);
    if (src == 7 || src == 15) return vec4(tex3, 0.0, 1.0);
    if (src == 4) return vec4(tex0, 0.0, 1.0);
    if (src == 12) return vec4(tex0, 0.0, 1.0);
    /* Conservative fallback for less common sources not wired yet. */
    return vec4(tex0, 0.0, 1.0);
}

vec2 applyTexGen(int idx, vec3 pos, vec3 nrm, vec2 tex0, vec2 tex1, vec2 tex2, vec2 tex3) {
    vec4 tc = texgenInput(u_texgen_src[idx], pos, nrm, tex0, tex1, tex2, tex3);
    if (u_texmtx_enable[idx] != 0) {
        return vec2(dot(u_texmtx_row0[idx], tc), dot(u_texmtx_row1[idx], tc));
    }
    return tc.xy;
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
    v_texcoord0 = applyTexGen(0, a_position, v_normal, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3);
    v_texcoord1 = applyTexGen(1, a_position, v_normal, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3);
    v_texcoord2 = applyTexGen(2, a_position, v_normal, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3);
    v_texcoord3 = applyTexGen(3, a_position, v_normal, a_texcoord0, a_texcoord1, a_texcoord2, a_texcoord3);
}
