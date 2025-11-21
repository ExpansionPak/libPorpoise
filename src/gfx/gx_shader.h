/*---------------------------------------------------------------------------*
  gx_shader.h - Basic OpenGL Shader for GX Rendering
 *---------------------------------------------------------------------------*/

#ifndef GX_SHADER_H
#define GX_SHADER_H

#include "gx_gl.h"

#ifdef __cplusplus
extern "C" {
#endif

void GXShaderInit(void);
void GXShaderShutdown(void);
GLuint GXShaderGetProgram(void);

#ifdef __cplusplus
}
#endif

#endif /* GX_SHADER_H */

