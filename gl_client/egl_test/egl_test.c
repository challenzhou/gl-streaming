/*
Copyright (c) 2013, Shodruky Rhyammer
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

  Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <linux/joystick.h>
#include <fcntl.h>
#include <math.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "glsutil.h"

#define g_vsize_pos 3
#define g_stride_pos (g_vsize_pos * sizeof(GLfloat))
#define g_vsize_nor 3
#define g_stride_nor (g_vsize_nor * sizeof(GLfloat))

EGLSurface surface_;
EGLContext context_;
EGLDisplay display_;

typedef struct
{
  union
  {
    struct
    {
      uint8_t r;
      uint8_t g;
      uint8_t b;
      uint8_t a;
    };
    uint32_t rgba;
  };
} color_t;

  
typedef struct
{
  float modelproj_mat[16];
  float nor_mat[16];
  uint32_t type;
  color_t color;
  float x;
  float y;
  float z;
  float dx;
  float dy;
  float dz;
  float rx;
  float ry;
  float rz;
  float drx;
  float dry;
  float drz;
} obj_3d_t;


static GLfloat light_pos[4] = {5.0, 5.0, 10.0, 1.0};
static obj_3d_t obj[1024];
static GLfloat proj_mat[16];
static float model_mat[16];
static float modelproj_mat[16];
static float nor_mat[16];
static float col[4];


static GLfloat vtx_cube[] =
{
  1.000000,1.000000,-1.000000,
  1.000000,-1.000000,-1.000000,
  -1.000000,-1.000000,-1.000000,
  1.000000,0.999999,1.000000,
  -1.000000,1.000000,1.000000,
  0.999999,-1.000001,1.000000,
  1.000000,1.000000,-1.000000,
  1.000000,0.999999,1.000000,
  1.000000,-1.000000,-1.000000,
  1.000000,-1.000000,-1.000000,
  0.999999,-1.000001,1.000000,
  -1.000000,-1.000000,-1.000000,
  -1.000000,-1.000000,-1.000000,
  -1.000000,-1.000000,1.000000,
  -1.000000,1.000000,1.000000,
  1.000000,0.999999,1.000000,
  1.000000,1.000000,-1.000000,
  -1.000000,1.000000,1.000000,
  -1.000000,1.000000,-1.000000,
  -1.000000,-1.000000,1.000000,
  1.000000,0.999999,1.000000,
  0.999999,-1.000001,1.000000,
  1.000000,-1.000000,-1.000000,
  -1.000000,-1.000000,1.000000,
  -1.000000,1.000000,-1.000000,
  -1.000000,1.000000,-1.000000,
};


static GLfloat nor_cube[] =
{
  0.000000,0.000000,-1.000000,
  0.000000,0.000000,-1.000000,
  0.000000,0.000000,-1.000000,
  -0.000000,-0.000000,1.000000,
  -0.000000,-0.000000,1.000000,
  -0.000000,-0.000000,1.000000,
  1.000000,0.000000,-0.000000,
  1.000000,0.000000,-0.000000,
  1.000000,0.000000,-0.000000,
  -0.000000,-1.000000,-0.000000,
  -0.000000,-1.000000,-0.000000,
  -0.000000,-1.000000,-0.000000,
  -1.000000,0.000000,-0.000000,
  -1.000000,0.000000,-0.000000,
  -1.000000,0.000000,-0.000000,
  0.000000,1.000000,0.000000,
  0.000000,1.000000,0.000000,
  0.000000,1.000000,0.000000,
  0.000000,0.000000,-1.000000,
  0.000000,-0.000000,1.000000,
  1.000000,-0.000001,0.000000,
  1.000000,-0.000001,0.000000,
  1.000000,-0.000001,0.000000,
  -0.000000,-1.000000,0.000000,
  -1.000000,0.000000,-0.000000,
  0.000000,1.000000,0.000000,
};


static GLushort ind_cube[] =
{
  0,1,2,
  3,4,5,
  6,7,8,
  9,10,11,
  12,13,14,
  15,16,17,
  18,0,2,
  4,19,5,
  20,21,22,
  10,23,11,
  24,12,14,
  16,25,17,
};


static const GLchar *VShader1 = 
  "attribute vec3 pos;\n"
  "attribute vec3 nor;\n"
  "uniform mat4 model_mat;\n"
  "uniform mat4 nor_mat;\n"
  "uniform vec4 light_pos;\n"
  "uniform vec4 col;\n"
  "varying vec4 fcol;\n"
  "void main(void)\n"
  "{\n"
  " vec3 eyenor = normalize(vec3(nor_mat * vec4(nor, 1.0)));\n"
  " vec3 lnor = normalize(light_pos.xyz);\n"
  " float diffuse = max(dot(eyenor, lnor), 0.0);\n"
  " fcol = vec4(vec3(diffuse * col), 1.0) ;\n"
  " gl_Position = model_mat * vec4(pos, 1.0);\n"
  "}\n";


static const GLchar *FShader1 = 
  "precision mediump float;\n"
  "varying vec4 fcol;\n"
  "void main(void)\n"
  "{\n"
  " gl_FragColor = fcol;\n"
  "}\n";


typedef struct
{
  uint32_t screen_width;
  uint32_t screen_height;
  GLuint vshader;
  GLuint fshader;
  GLuint program;
  GLuint vbo_pos;
  GLuint vbo_nor;
  GLuint vbo_ind;
  GLint vloc_pos;
  GLint vloc_nor;
  GLuint uloc_model;
  GLuint uloc_nor;
  GLuint uloc_light;
  GLuint uloc_col;
  GLfloat* vtx_pos;
  GLfloat* vtx_nor;
} graphics_context_t;



static inline float randf(void)
{
  return (float)rand() / (float)RAND_MAX;
}

/*
float get_diff_time(struct timeval start, struct timeval end)
{
  float dt = (float)(end.tv_sec - start.tv_sec) + (float)(end.tv_usec - start.tv_usec) * 0.000001f;
  return dt;
}
*/

bool CreateCtx(void) {

    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config = nullptr;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, nullptr, nullptr);

    /* Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            r == 8 && g == 8 && b == 8 && d == 0 ) {

            config = supportedConfigs[i];
            break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }

    if (config == nullptr) {
        LOGW("Unable to initialize EGLConfig");
        return false;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
      // Create GL3 Context
    std::vector<EGLint> attributes;
    attributes.resize(0);
    attributes.push_back(EGL_CONTEXT_CLIENT_VERSION);
    attributes.push_back(3);
    attributes.push_back(EGL_NONE);
    surface = eglCreateWindowSurface(display, config, globalAppState->window, nullptr);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT , attributes.data());

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return false;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    display_ = display;
    eglContext_ = context;
    surface_ = surface;
    renderTargetWidth_ = w;
    renderTargetHeight_ = h;

    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }

    return true;

}


GLuint create_vbo(GLenum target, void *vtx, size_t size)
{
  static GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(target, vbo);
  glBufferData(target, size, vtx, GL_STATIC_DRAW);
  glBindBuffer(target, 0);
  return vbo;
}


void release_vbo(GLenum target, GLuint vbo)
{
  glBindBuffer(target, 0);
  glDeleteBuffers(1, &vbo);
}


void init_shader(graphics_context_t *gc, const GLchar *vs, const GLchar *fs)
{
  gc->vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(gc->vshader, 1, &vs, 0);
  glCompileShader(gc->vshader);
  gc->fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(gc->fshader, 1, &fs, 0);
  glCompileShader(gc->fshader);

  gc->program = glCreateProgram();
  glAttachShader(gc->program, gc->vshader);
  glAttachShader(gc->program, gc->fshader);
  glLinkProgram(gc->program);
}

void DestroyCtx()
{
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (eglContext_ != EGL_NO_CONTEXT) {
            eglDestroyContext(display_, eglContext_);
        }
        if (surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(display_, surface_);
        }
        eglTerminate(display_);
    }

  	eglContext_ = EGL_NO_CONTEXT;
  	surface_ = EGL_NO_SURFACE;
  	display_ = EGL_NO_DISPLAY;
}

void release_shader(graphics_context_t *gc)
{
  glDeleteProgram(gc->program);
  glDeleteShader(gc->vshader);
  glDeleteShader(gc->fshader);
}


void init_gl(graphics_context_t *gc)
{
  init_shader(gc, VShader1, FShader1);
  gc->vbo_pos = create_vbo(GL_ARRAY_BUFFER, vtx_cube, sizeof(vtx_cube));
  gc->vbo_nor = create_vbo(GL_ARRAY_BUFFER, nor_cube, sizeof(nor_cube));
  gc->vbo_ind = create_vbo(GL_ELEMENT_ARRAY_BUFFER, ind_cube, sizeof(ind_cube));
  gc->vloc_pos = glGetAttribLocation(gc->program, "pos");
  gc->vloc_nor = glGetAttribLocation(gc->program, "nor");
  gc->uloc_model = glGetUniformLocation(gc->program, "model_mat");
  gc->uloc_nor = glGetUniformLocation(gc->program, "nor_mat");
  gc->uloc_light = glGetUniformLocation(gc->program, "light_pos");
  gc->uloc_col = glGetUniformLocation(gc->program, "col");

  glBindBuffer(GL_ARRAY_BUFFER, gc->vbo_pos);
  glVertexAttribPointer(gc->vloc_pos, g_vsize_pos, GL_FLOAT, GL_FALSE, g_stride_pos, (GLfloat *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindBuffer(GL_ARRAY_BUFFER, gc->vbo_nor);
  glVertexAttribPointer(gc->vloc_nor, g_vsize_nor, GL_FLOAT, GL_FALSE, g_stride_nor, (GLfloat *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindFramebuffer(GL_FRAMEBUFFER,0);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, gc->screen_width, gc->screen_height);
  glUseProgram(gc->program);

  glDisable(GL_BLEND);

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}


void release_gl(graphics_context_t *gc)
{
  release_shader(gc);
  release_vbo(GL_ARRAY_BUFFER, gc->vbo_pos);
  release_vbo(GL_ARRAY_BUFFER, gc->vbo_nor);
  release_vbo(GL_ELEMENT_ARRAY_BUFFER, gc->vbo_ind);
}



void glclient_draw( )
{
  static graphics_context_t gc;

  static struct js_event joy;
  int joy_fd;
  static char button[32];

#if 0
  gc.screen_width = glsc_global.screen_width;
  gc.screen_height = glsc_global.screen_height;
  printf("width:%d height:%d\n",glsc_global.screen_width,glsc_global.screen_height);
#endif
  init_gl(&gc);

  float aspect = (float)gc.screen_width / (float)gc.screen_height;

  mat_perspective(proj_mat, aspect, 1.0f, 1024.0f, 60.0f);
  glUniform4fv(gc.uloc_light, 1, light_pos);

  int j;
  for (j = 0; j < 1024; j++)
  {
    obj[j].x = randf() * 60.0f - 30.0f;
    obj[j].y = randf() * 60.0f - 30.0f;
    obj[j].z = randf() * 300.0f - 300.0f;
    obj[j].dx = randf() * 0.0f - 0.00f;
    obj[j].dy = randf() * 0.0f - 0.00f;
    obj[j].dz = randf() * 1.3f - 0.3f;
    if (fabs(obj[j].dz) < 0.01f)
    {
      obj[j].dz = 0.01f;
    }
    obj[j].rx = randf() * 6.28;
    obj[j].ry = randf() * 6.28;
    obj[j].rz = randf() * 6.28;
    obj[j].drx = randf() * 0.1f - 0.05f;
    obj[j].dry = randf() * 0.1f - 0.05f;
    obj[j].drz = randf() * 0.1f - 0.05f;
    obj[j].color.r = (uint8_t)(randf() * 255.5f);
    obj[j].color.g = (uint8_t)(randf() * 255.5f);
    obj[j].color.b = (uint8_t)(randf() * 255.5f);
    obj[j].color.a = (uint8_t)255;
  }

  float x = 0.0f;
  float y = 0.0f;
  float rx = 0.0f;
  float ry = 0.0f;

  int i;
  for (i = 0; i < 432000; i++)
  {
    struct timeval times, timee;
    gettimeofday(&times, NULL);

    if (joy_fd != -1)
    {
      while (read(joy_fd, &joy, sizeof(struct js_event)) == sizeof(struct js_event))
      {
        if ((joy.type & JS_EVENT_BUTTON) == JS_EVENT_BUTTON)
        {
          button[joy.number] = joy.value;
        }
      }

      if (button[4] > 0)
      {
        y += 0.01f;
      }
      if (button[6] > 0)
      {
        y += -0.01f;
      }
      if (button[5] > 0)
      {
        x += 0.01f * aspect;
      }
      if (button[7] > 0)
      {
        x += -0.01f * aspect;
      }
      if (button[12] > 0)
      {
        rx += -0.01f;
      }
      if (button[13] > 0)
      {
        ry += 0.01f;
      }
      if (button[14] > 0)
      {
        rx += 0.01f;
      }
      if (button[15] > 0)
      {
        ry += -0.01f;
      }
    }

    glUseProgram(gc.program);
    glBindBuffer(GL_ARRAY_BUFFER, gc.vbo_pos);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gc.vbo_ind);
    glEnableVertexAttribArray(gc.vloc_pos);
    glEnableVertexAttribArray(gc.vloc_nor);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (j = 0; j < 240; j++)
    {
      obj[j].x += obj[j].dx;
      obj[j].y += obj[j].dy;
      obj[j].z += obj[j].dz + y;
      if (obj[j].z > 1.0f)
      {
        obj[j].x = randf() * 60.0f - 30.0f;
        obj[j].y = randf() * 60.0f - 30.0f;
        obj[j].z = -300.0f;
      }
      if (obj[j].z < -400.0f)
      {
        obj[j].x = randf() * 60.0f - 30.0f;
        obj[j].y = randf() * 60.0f - 30.0f;
        obj[j].z = 1.0f;
      }
      obj[j].rx += obj[j].drx;
      obj[j].ry += obj[j].dry;
      obj[j].rz += obj[j].drz;

      mat_identity(model_mat);
      mat_translate(model_mat, obj[j].x, obj[j].y, obj[j].z);
      mat_rotate_x(model_mat, obj[j].rx);
      mat_rotate_y(model_mat, obj[j].ry);
      mat_rotate_z(model_mat, obj[j].rz);

      mat_copy(nor_mat, model_mat);
      mat_invert(nor_mat);
      mat_transpose(nor_mat);
      glUniformMatrix4fv(gc.uloc_nor, 1, GL_FALSE, nor_mat);
      mat_copy(obj[j].nor_mat, nor_mat);

      mat_copy(modelproj_mat, proj_mat);
      mat_mul(modelproj_mat, model_mat);
      glUniformMatrix4fv(gc.uloc_model, 1, GL_FALSE, modelproj_mat);
      mat_copy(obj[j].modelproj_mat, modelproj_mat);

      col[0] = (float)obj[j].color.r * 0.00392156862745f;
      col[1] = (float)obj[j].color.g * 0.00392156862745f;
      col[2] = (float)obj[j].color.b * 0.00392156862745f;
      col[3] = (float)obj[j].color.a * 0.00392156862745f;
      glUniform4fv(gc.uloc_col, 1, col);

      glDrawElements(GL_TRIANGLES, sizeof(ind_cube) / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    }

    eglSwapBuffers(display_, surface_);

    glDisableVertexAttribArray(gc.vloc_nor);
    glDisableVertexAttribArray(gc.vloc_pos);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    gettimeofday(&timee, NULL);
    //printf("%d:%f ms ", i, get_diff_time(times, timee) * 1000.0f);
  }
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  release_gl(&gc);
}


int main(int argc, char * argv[])
{


  CreateCtx();
  glclient_draw();
  DestroyCtx();

  return 0;
}
