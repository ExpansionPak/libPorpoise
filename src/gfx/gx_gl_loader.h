/*---------------------------------------------------------------------------*
  gx_gl_loader.h - OpenGL Function Loader
 *---------------------------------------------------------------------------*/

#ifndef GX_GL_LOADER_H
#define GX_GL_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Load OpenGL 3.3 functions at runtime (Windows only) */
void GXGLLoadFunctions(void);

#ifdef __cplusplus
}
#endif

#endif /* GX_GL_LOADER_H */

