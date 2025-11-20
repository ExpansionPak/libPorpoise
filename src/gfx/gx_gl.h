/*---------------------------------------------------------------------------*
  gx_gl.h - OpenGL Header Wrapper
  
  Ensures proper OpenGL header inclusion across platforms
 *---------------------------------------------------------------------------*/

#ifndef GX_GL_H
#define GX_GL_H

/* Windows requires windows.h before GL headers */
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

/* OpenGL headers - platform specific */
#ifdef _WIN32
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#endif /* GX_GL_H */

