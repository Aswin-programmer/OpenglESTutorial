#pragma once

#include <cassert>

#include <GLES/egl.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include "Logger.h"

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
};

