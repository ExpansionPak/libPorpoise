/*---------------------------------------------------------------------------*
  gx_shader.c - Basic OpenGL Shader for GX Rendering
  
  Provides a simple shader program for rendering GX vertices
 *---------------------------------------------------------------------------*/

#include "gx_gl.h"
#include "gx_shader.h"
#include <dolphin/os.h>
#include <stdlib.h>
#include <string.h>

static GLuint s_shaderProgram = 0;
static GLuint s_vertexShader = 0;
static GLuint s_fragmentShader = 0;

/* Basic vertex shader - passes through position and color */
static const char* s_vertexShaderSource = 
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPosition;\n"
    "layout(location = 1) in vec4 aColor;\n"
    "uniform mat4 uProjection;\n"
    "uniform mat4 uModelView;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    /* OpenGL matrices are column-major, so matrix * vector is correct */\n"
    "    /* When we pass row-major with GL_FALSE, OpenGL transposes it to column-major */\n"
    "    vec4 pos = uModelView * vec4(aPosition, 1.0);\n"
    "    gl_Position = uProjection * pos;\n"
    "    vColor = aColor;\n"
    "}\n";

/* Basic fragment shader - outputs color */
static const char* s_fragmentShaderSource =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vColor;\n"
    "}\n";

static GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        OSReport("[GX ERROR] Failed to create shader\n");
        return 0;
    }
    
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        OSReport("[GX ERROR] Shader compilation failed: %s\n", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

static GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    if (program == 0) {
        OSReport("[GX ERROR] Failed to create shader program\n");
        return 0;
    }
    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        OSReport("[GX ERROR] Shader program linking failed: %s\n", infoLog);
        glDeleteProgram(program);
        return 0;
    }
    
    return program;
}

void GXShaderInit(void) {
    if (s_shaderProgram != 0) {
        return;  /* Already initialized */
    }
    
    s_vertexShader = CompileShader(GL_VERTEX_SHADER, s_vertexShaderSource);
    if (s_vertexShader == 0) {
        return;
    }
    
    s_fragmentShader = CompileShader(GL_FRAGMENT_SHADER, s_fragmentShaderSource);
    if (s_fragmentShader == 0) {
        glDeleteShader(s_vertexShader);
        s_vertexShader = 0;
        return;
    }
    
    s_shaderProgram = LinkProgram(s_vertexShader, s_fragmentShader);
    if (s_shaderProgram == 0) {
        glDeleteShader(s_vertexShader);
        glDeleteShader(s_fragmentShader);
        s_vertexShader = 0;
        s_fragmentShader = 0;
        return;
    }
    
    OSReport("[GX INFO] Basic shader program created successfully\n");
}

void GXShaderShutdown(void) {
    if (s_shaderProgram != 0) {
        glDeleteProgram(s_shaderProgram);
        s_shaderProgram = 0;
    }
    if (s_vertexShader != 0) {
        glDeleteShader(s_vertexShader);
        s_vertexShader = 0;
    }
    if (s_fragmentShader != 0) {
        glDeleteShader(s_fragmentShader);
        s_fragmentShader = 0;
    }
}

GLuint GXShaderGetProgram(void) {
    return s_shaderProgram;
}

