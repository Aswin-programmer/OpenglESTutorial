#pragma once

#include <cassert>
#include <chrono>

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Logger.h"

struct COLOR
{
    COLOR(int r_, int g_, int b_)
        :
        r{r_},
        g{g_},
        b{b_}
    {

    }
    int r, g, b;
};

class Renderer {
public:
    Renderer(android_app *app);
    ~Renderer();

    void Do_Frame();

private:
    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    GLuint vao, vbo, ebo;
    GLuint program;
    GLint projection_location, model_location;

    COLOR color_test;
};

