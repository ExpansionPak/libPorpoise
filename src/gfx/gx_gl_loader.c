/*---------------------------------------------------------------------------*
  gx_gl_loader.c - OpenGL Function Loader
  
  Loads OpenGL 3.3 functions at runtime on Windows using wglGetProcAddress
 *---------------------------------------------------------------------------*/

#include "gx_gl.h"
#include <dolphin/os.h>

#ifdef _WIN32
#include <windows.h>

/* Function pointers for OpenGL 3.3 functions */
static PFNGLGENBUFFERSPROC glGenBuffers_ptr = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer_ptr = NULL;
static PFNGLBUFFERDATAPROC glBufferData_ptr = NULL;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers_ptr = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray_ptr = NULL;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray_ptr = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer_ptr = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram_ptr = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ptr = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv_ptr = NULL;
static PFNGLCREATESHADERPROC glCreateShader_ptr = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource_ptr = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader_ptr = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv_ptr = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ptr = NULL;
static PFNGLDELETESHADERPROC glDeleteShader_ptr = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram_ptr = NULL;
static PFNGLATTACHSHADERPROC glAttachShader_ptr = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram_ptr = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_ptr = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ptr = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_ptr = NULL;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ptr = NULL;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ptr = NULL;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays_ptr = NULL;

/* Load all OpenGL 3.3 functions */
void GXGLLoadFunctions(void) {
    glGenBuffers_ptr = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
    glBindBuffer_ptr = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
    glBufferData_ptr = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
    glDeleteBuffers_ptr = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
    glEnableVertexAttribArray_ptr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray_ptr = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");
    glVertexAttribPointer_ptr = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
    glUseProgram_ptr = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
    glCreateShader_ptr = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource_ptr = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader_ptr = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glDeleteShader_ptr = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glAttachShader_ptr = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
    glGenVertexArrays_ptr = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
    glDeleteVertexArrays_ptr = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
    
    /* Check if all functions loaded successfully */
    if (!glGenBuffers_ptr || !glBindBuffer_ptr || !glBufferData_ptr || !glDeleteBuffers_ptr ||
        !glEnableVertexAttribArray_ptr || !glDisableVertexAttribArray_ptr || !glVertexAttribPointer_ptr ||
        !glUseProgram_ptr || !glGetUniformLocation_ptr || !glUniformMatrix4fv_ptr ||
        !glCreateShader_ptr || !glShaderSource_ptr || !glCompileShader_ptr ||
        !glGetShaderiv_ptr || !glGetShaderInfoLog_ptr || !glDeleteShader_ptr ||
        !glCreateProgram_ptr || !glAttachShader_ptr || !glLinkProgram_ptr ||
        !glGetProgramiv_ptr || !glGetProgramInfoLog_ptr || !glDeleteProgram_ptr ||
        !glGenVertexArrays_ptr || !glBindVertexArray_ptr || !glDeleteVertexArrays_ptr) {
        OSReport("[GX WARN] Some OpenGL functions failed to load\n");
    } else {
        OSReport("[GX INFO] OpenGL function loading complete\n");
    }
}

/* Wrapper functions that call the loaded function pointers */
void glGenBuffers(GLsizei n, GLuint *buffers) { 
    if (glGenBuffers_ptr) glGenBuffers_ptr(n, buffers); 
}
void glBindBuffer(GLenum target, GLuint buffer) { 
    if (glBindBuffer_ptr) glBindBuffer_ptr(target, buffer); 
}
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) { 
    if (glBufferData_ptr) glBufferData_ptr(target, size, data, usage); 
}
void glDeleteBuffers(GLsizei n, const GLuint *buffers) { 
    if (glDeleteBuffers_ptr) glDeleteBuffers_ptr(n, buffers); 
}
void glEnableVertexAttribArray(GLuint index) { 
    if (glEnableVertexAttribArray_ptr) glEnableVertexAttribArray_ptr(index); 
}
void glDisableVertexAttribArray(GLuint index) { 
    if (glDisableVertexAttribArray_ptr) glDisableVertexAttribArray_ptr(index); 
}
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) { 
    if (glVertexAttribPointer_ptr) glVertexAttribPointer_ptr(index, size, type, normalized, stride, pointer); 
}
void glUseProgram(GLuint program) { 
    if (glUseProgram_ptr) glUseProgram_ptr(program); 
}
GLint glGetUniformLocation(GLuint program, const GLchar *name) { 
    if (!glGetUniformLocation_ptr) return -1;
    return glGetUniformLocation_ptr(program, name); 
}
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { 
    if (glUniformMatrix4fv_ptr) glUniformMatrix4fv_ptr(location, count, transpose, value); 
}
GLuint glCreateShader(GLenum type) { 
    GLuint result = 0;
    if (glCreateShader_ptr) {
        result = glCreateShader_ptr(type);
    }
    return result;
}
void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) { 
    if (glShaderSource_ptr) glShaderSource_ptr(shader, count, string, length); 
}
void glCompileShader(GLuint shader) { 
    if (glCompileShader_ptr) glCompileShader_ptr(shader); 
}
void glGetShaderiv(GLuint shader, GLenum pname, GLint* params) { 
    if (glGetShaderiv_ptr) glGetShaderiv_ptr(shader, pname, params); 
}
void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { 
    if (glGetShaderInfoLog_ptr) glGetShaderInfoLog_ptr(shader, bufSize, length, infoLog); 
}
void glDeleteShader(GLuint shader) { 
    if (glDeleteShader_ptr) glDeleteShader_ptr(shader); 
}
GLuint glCreateProgram(void) { 
    GLuint result = 0;
    if (glCreateProgram_ptr) {
        result = glCreateProgram_ptr();
    }
    return result;
}
void glAttachShader(GLuint program, GLuint shader) { 
    if (glAttachShader_ptr) glAttachShader_ptr(program, shader); 
}
void glLinkProgram(GLuint program) { 
    if (glLinkProgram_ptr) glLinkProgram_ptr(program); 
}
void glGetProgramiv(GLuint program, GLenum pname, GLint* params) { 
    if (glGetProgramiv_ptr) glGetProgramiv_ptr(program, pname, params); 
}
void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) { 
    if (glGetProgramInfoLog_ptr) glGetProgramInfoLog_ptr(program, bufSize, length, infoLog); 
}
void glDeleteProgram(GLuint program) { 
    if (glDeleteProgram_ptr) glDeleteProgram_ptr(program); 
}
void glGenVertexArrays(GLsizei n, GLuint *arrays) { 
    if (glGenVertexArrays_ptr) glGenVertexArrays_ptr(n, arrays); 
}
void glBindVertexArray(GLuint array) { 
    if (glBindVertexArray_ptr) glBindVertexArray_ptr(array); 
}
void glDeleteVertexArrays(GLsizei n, const GLuint *arrays) { 
    if (glDeleteVertexArrays_ptr) glDeleteVertexArrays_ptr(n, arrays); 
}

#else
/* Non-Windows platforms: functions should be available directly */
void GXGLLoadFunctions(void) {
    /* No-op on non-Windows platforms */
}
#endif

